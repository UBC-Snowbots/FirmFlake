// Tests for the comms serialization library.
//
// Build (Catch2 v3):
//   g++ -std=c++17 test_comms.cpp comms.o -lCatch2Main -lCatch2 -o test_comms
// or with CMake, link against Catch2::Catch2WithMain.
//
// Scope is deliberately narrow: prove the library builds, and prove that a
// representative message from each shape (multi-word, single-word, mixed
// int/float, packed bytes) survives every transport round trip intact.

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include <cstring>
#include <vector>
#include <iomanip>

extern "C" {
#include <comms.h>
}

using Catch::Matchers::WithinRel;

namespace {

constexpr uint16_t kTestDeviceId = DEVICE_ID__LED_PANEL_BASE + 3;

// encode() refuses to run until a device id is set, and that id lives in a
// file-static inside comms.c. Every test that encodes sets it explicitly, so
// cases stay independent of execution order (Catch2 can run with --order rand).
void use_test_device_id() { comms_set_device_id(kTestDeviceId); }

msg_t make_msg(uint16_t type)
{
    msg_t m{};
    m.type = type;
    return m;
}

} // namespace

// ---------------------------------------------------------------- device id

TEST_CASE("device id is masked to 13 bits on set", "[comms][id]")
{
    comms_set_device_id(0xFFFF);
    REQUIRE(comms_get_device_id() == COMMS_DEVICE_ID_MASK);

    use_test_device_id();
    REQUIRE(comms_get_device_id() == kTestDeviceId);
}

TEST_CASE("encode refuses to run before a device id is set", "[comms][id]")
{
    comms_set_device_id(COMMS_INVALID_DEVICE_ID);

    msg_t msg = make_msg(MSG_TYPE__PING);
    encoded_msg_t enc{};
    REQUIRE_FALSE(encode(&msg, &enc));

    use_test_device_id(); // restore, so a randomized order can't poison later cases
}

TEST_CASE("decode recovers the sender id of the encoding device", "[comms][id]")
{
    constexpr uint16_t kOtherDevice = DEVICE_ID__CONTROL_BASE_SUC;
    comms_set_device_id(kOtherDevice);

    msg_t in = make_msg(MSG_TYPE__HEARTBEAT);
    in.payload.heartbeat.timestamp_ms = 1234u;

    encoded_msg_t enc{};
    REQUIRE(encode(&in, &enc));

    msg_t out{};
    REQUIRE(decode(&enc, &out));
    REQUIRE(out.sender_id == kOtherDevice);

    use_test_device_id();
}

// ------------------------------------------------------- payload round trips

TEST_CASE("ping round trips through encode/decode", "[comms][payload]")
{
    use_test_device_id();

    msg_t in = make_msg(MSG_TYPE__PING);
    in.payload.ping.reserved_1 = PING_RESERVED_1;
    in.payload.ping.reserved_2 = PING_RESERVED_2;

    encoded_msg_t enc{};
    REQUIRE(encode(&in, &enc));
    REQUIRE(enc.payload_len == 2u * sizeof(uint32_t));

    msg_t out{};
    REQUIRE(decode(&enc, &out));
    REQUIRE(out.type == MSG_TYPE__PING);
    REQUIRE(out.sender_id == kTestDeviceId);
    REQUIRE(out.payload.ping.reserved_1 == PING_RESERVED_1);
    REQUIRE(out.payload.ping.reserved_2 == PING_RESERVED_2);
}

TEST_CASE("heartbeat round trips through encode/decode", "[comms][payload]")
{
    use_test_device_id();

    msg_t in = make_msg(MSG_TYPE__HEARTBEAT);
    in.payload.heartbeat.timestamp_ms = 0xDEADBEEFu;

    encoded_msg_t enc{};
    REQUIRE(encode(&in, &enc));
    REQUIRE(enc.payload_len == sizeof(uint32_t));

    msg_t out{};
    REQUIRE(decode(&enc, &out));
    REQUIRE(out.type == MSG_TYPE__HEARTBEAT);
    REQUIRE(out.payload.heartbeat.timestamp_ms == 0xDEADBEEFu);
}

TEST_CASE("led pwm command round trips, mixed uint8 + float", "[comms][payload]")
{
    use_test_device_id();

    // This is the interesting one: a 1-byte field followed by a 4-byte float,
    // which is exactly where struct padding would silently corrupt a naive
    // memcpy-the-whole-struct implementation.
    msg_t in = make_msg(MSG_TYPE__SET_LED_PWM);
    in.payload.led_panel_cmd.led_panel_index = 7u;
    in.payload.led_panel_cmd.panel_percent   = 42.5f;

    encoded_msg_t enc{};
    REQUIRE(encode(&in, &enc));
    REQUIRE(enc.payload_len == sizeof(uint8_t) + sizeof(float));

    msg_t out{};
    REQUIRE(decode(&enc, &out));
    REQUIRE(out.type == MSG_TYPE__SET_LED_PWM);
    REQUIRE(out.payload.led_panel_cmd.led_panel_index == 7u);
    REQUIRE_THAT(out.payload.led_panel_cmd.panel_percent, WithinRel(42.5f));

    // NOTE: is_response is a member of LedPanelCMD_t but is neither packed nor
    // counted in expected_payload_len(), so it does NOT survive the wire.
    // Anything downstream (the ROS bridge included) must not rely on it until
    // it is either serialized or removed from the struct.
}

TEST_CASE("led pwm survives fractional values bit-exactly", "[comms][payload]")
{
    use_test_device_id();

    const float values[] = {0.0f, 0.1f, 33.333f, 99.9f, 100.0f};

    for (float v : values) {
        msg_t in = make_msg(MSG_TYPE__SET_LED_PWM);
        in.payload.led_panel_cmd.led_panel_index = 0u;
        in.payload.led_panel_cmd.panel_percent   = v;

        encoded_msg_t enc{};
        REQUIRE(encode(&in, &enc));
        msg_t out{};
        REQUIRE(decode(&enc, &out));

        // Serialization is a byte copy, so this should be exact, not merely close.
        REQUIRE(std::memcmp(&out.payload.led_panel_cmd.panel_percent, &v, sizeof(float)) == 0);
    }
}

TEST_CASE("hardware error round trips through encode/decode", "[comms][payload]")
{
    use_test_device_id();

    msg_t in = make_msg(MSG_TYPE__HARDWARE_ERROR);
    in.payload.hardware_error.error_type = HARDWARE_ERROR_TYPE__INCORRECT_FIRMWARE_VERSION;
    in.payload.hardware_error.info       = 0x5Au;

    encoded_msg_t enc{};
    REQUIRE(encode(&in, &enc));
    REQUIRE(enc.payload_len == 2u);

    msg_t out{};
    REQUIRE(decode(&enc, &out));
    REQUIRE(out.payload.hardware_error.error_type == HARDWARE_ERROR_TYPE__INCORRECT_FIRMWARE_VERSION);
    REQUIRE(out.payload.hardware_error.info == 0x5Au);
}

TEST_CASE("unknown message types are rejected", "[comms][payload]")
{
    use_test_device_id();

    msg_t unknown = make_msg(MSG_TYPE__UNKNOWN);
    encoded_msg_t enc{};
    REQUIRE_FALSE(encode(&unknown, &enc));

    msg_t past_end = make_msg(NUM_MSG_TYPES);
    REQUIRE_FALSE(encode(&past_end, &enc));

    // And on the way back in: an id carrying a type this build has never heard of.
    encoded_msg_t bogus{};
    bogus.id = (static_cast<uint32_t>(kTestDeviceId) << COMMS_DEVICE_ID_SHIFT) | 0xBEEFu;
    bogus.payload_len = 4u;
    msg_t out{};
    REQUIRE_FALSE(decode(&bogus, &out));
}

// ------------------------------------------------------------ CAN transport

TEST_CASE("payload survives the CAN round trip", "[comms][can]")
{
    use_test_device_id();

    msg_t in = make_msg(MSG_TYPE__SET_LED_PWM);
    in.payload.led_panel_cmd.led_panel_index = 12u;
    in.payload.led_panel_cmd.panel_percent   = 88.25f;

    encoded_msg_t enc{};
    REQUIRE(encode(&in, &enc));

    can_msg_t frame{};
    REQUIRE(convert_to_can(&enc, &frame));
    REQUIRE(frame.payload_len == enc.payload_len);
    REQUIRE(frame.id == (enc.id & COMMS_CAN_EXT_ID_MASK));
    REQUIRE(frame.id <= COMMS_CAN_EXT_ID_MASK); // must fit a 29-bit extended id

    encoded_msg_t back{};
    REQUIRE(convert_from_can(&frame, &back));
    REQUIRE(back.id == enc.id);
    REQUIRE(back.payload_len == enc.payload_len);

    msg_t out{};
    REQUIRE(decode(&back, &out));
    REQUIRE(out.type == MSG_TYPE__SET_LED_PWM);
    REQUIRE(out.sender_id == kTestDeviceId);
    REQUIRE(out.payload.led_panel_cmd.led_panel_index == 12u);
    REQUIRE_THAT(out.payload.led_panel_cmd.panel_percent, WithinRel(88.25f));
}

TEST_CASE("oversized payloads are refused by the CAN converters", "[comms][can]")
{
    encoded_msg_t enc{};
    enc.id = 0x1234u;
    enc.payload_len = MAX_MESSAGE_PAYLOAD_SIZE_BYTES + 1u;

    can_msg_t frame{};
    REQUIRE_FALSE(convert_to_can(&enc, &frame));

    can_msg_t oversized{};
    oversized.payload_len = MAX_MESSAGE_PAYLOAD_SIZE_BYTES + 1u;
    encoded_msg_t back{};
    REQUIRE_FALSE(convert_from_can(&oversized, &back));
}

// ------------------------------------------------------------ raw transport

TEST_CASE("payload survives the raw byte round trip", "[comms][raw]")
{
    use_test_device_id();

    msg_t in = make_msg(MSG_TYPE__PING);
    in.payload.ping.reserved_1 = PING_RESERVED_1;
    in.payload.ping.reserved_2 = PING_RESERVED_2;

    encoded_msg_t enc{};
    REQUIRE(encode(&in, &enc));

    uint8_t raw[COMMS_RAW_MSG_SIZE] = {0};
    REQUIRE(convert_to_raw(&enc, raw));

    encoded_msg_t back{};
    REQUIRE(convert_from_raw(raw, &back));
    REQUIRE(back.id == enc.id);
    REQUIRE(back.payload_len == enc.payload_len);
    REQUIRE(std::memcmp(back.payload, enc.payload, enc.payload_len) == 0);

    msg_t out{};
    REQUIRE(decode(&back, &out));
    REQUIRE(out.type == MSG_TYPE__PING);
    REQUIRE(out.sender_id == kTestDeviceId);
    REQUIRE(out.payload.ping.reserved_1 == PING_RESERVED_1);
    REQUIRE(out.payload.ping.reserved_2 == PING_RESERVED_2);
}

TEST_CASE("raw header layout is little-endian device id then type", "[comms][raw]")
{
    use_test_device_id();

    msg_t in = make_msg(MSG_TYPE__HEARTBEAT);
    in.payload.heartbeat.timestamp_ms = 0x01020304u;

    encoded_msg_t enc{};
    REQUIRE(encode(&in, &enc));

    uint8_t raw[COMMS_RAW_MSG_SIZE] = {0};
    REQUIRE(convert_to_raw(&enc, raw));

    // Pinning the wire format down explicitly: anything decoding these bytes on
    // the other end (a ROS bridge, a Python test rig) depends on this layout.
    uint16_t dev = 0, type = 0;
    std::memcpy(&dev, raw + 0, sizeof(dev));
    std::memcpy(&type, raw + 2, sizeof(type));
    REQUIRE(dev == kTestDeviceId);
    REQUIRE(type == MSG_TYPE__HEARTBEAT);

    uint32_t ts = 0;
    std::memcpy(&ts, raw + COMMS_HEADER_SIZE_BYTES, sizeof(ts));
    REQUIRE(ts == 0x01020304u);

    // Bytes past the payload are zero-padded to the fixed frame width.
    for (size_t i = COMMS_HEADER_SIZE_BYTES + sizeof(uint32_t); i < COMMS_RAW_MSG_SIZE; ++i) {
        REQUIRE(raw[i] == 0u);
    }
}

TEST_CASE("raw frames with an unknown type are rejected", "[comms][raw]")
{
    uint8_t raw[COMMS_RAW_MSG_SIZE] = {0};
    uint16_t dev  = kTestDeviceId;
    uint16_t type = 0xBEEFu; // not in expected_payload_len()
    std::memcpy(raw + 0, &dev, sizeof(dev));
    std::memcpy(raw + 2, &type, sizeof(type));

    encoded_msg_t out{};
    REQUIRE_FALSE(convert_from_raw(raw, &out));
}

// ------------------------------------------------------------- null guards

TEST_CASE("every entry point rejects null pointers", "[comms][guards]")
{
    use_test_device_id();

    msg_t msg = make_msg(MSG_TYPE__PING);
    encoded_msg_t enc{};
    can_msg_t frame{};
    uint8_t raw[COMMS_RAW_MSG_SIZE] = {0};

    REQUIRE_FALSE(encode(nullptr, &enc));
    REQUIRE_FALSE(encode(&msg, nullptr));
    REQUIRE_FALSE(decode(nullptr, &msg));
    REQUIRE_FALSE(decode(&enc, nullptr));
    REQUIRE_FALSE(convert_to_can(nullptr, &frame));
    REQUIRE_FALSE(convert_to_can(&enc, nullptr));
    REQUIRE_FALSE(convert_from_can(nullptr, &enc));
    REQUIRE_FALSE(convert_from_can(&frame, nullptr));
    REQUIRE_FALSE(convert_to_raw(nullptr, raw));
    REQUIRE_FALSE(convert_to_raw(&enc, nullptr));
    REQUIRE_FALSE(convert_from_raw(nullptr, &enc));
    REQUIRE_FALSE(convert_from_raw(raw, nullptr));
}

// --------------------------------------------------------------- known gap

// decode() never checks payload_len against the length the type actually
// requires, so a short or corrupt frame unpacks whatever happens to be in the
// buffer and reports success. Tagged [!shouldfail] so it passes today and
// starts failing loudly the moment someone adds the length check, at which
// point delete the tag.
TEST_CASE("decode rejects a truncated payload", "[comms][known-gap][!shouldfail]")
{
    use_test_device_id();

    msg_t in = make_msg(MSG_TYPE__PING);
    in.payload.ping.reserved_1 = PING_RESERVED_1;
    in.payload.ping.reserved_2 = PING_RESERVED_2;

    encoded_msg_t enc{};
    REQUIRE(encode(&in, &enc));

    enc.payload_len = 3; // claim far less than PING's required 8 bytes

    msg_t out{};
    REQUIRE_FALSE(decode(&enc, &out));
}

// -------------------------------------------------------------- COBS
// lowkvibe coded testcases based on https://en.wikipedia.org/wiki/Consistent_Overhead_Byte_Stuffing#Encoding_examples
TEST_CASE("COBS matches the Wikipedia encoding examples (implementation + round trip test)", "[comms][cobs]")
{
    const std::vector<std::vector<uint8_t>> unencoded = {
        {0x00},
        {0x00, 0x00},
        {0x00, 0x11, 0x00},
        {0x11, 0x22, 0x00, 0x33},
        {0x11, 0x22, 0x33, 0x44},
        {0x11, 0x00, 0x00, 0x00},
    };

    const std::vector<std::vector<uint8_t>> encoded = {
        {0x01, 0x01, 0x00},
        {0x01, 0x01, 0x01, 0x00},
        {0x01, 0x02, 0x11, 0x01, 0x00},
        {0x03, 0x11, 0x22, 0x02, 0x33, 0x00},
        {0x05, 0x11, 0x22, 0x33, 0x44, 0x00},
        {0x02, 0x11, 0x01, 0x01, 0x01, 0x00},
    };

    for (size_t i = 0; i < unencoded.size(); ++i) {
        std::vector<uint8_t> encoded_output(encoded[i].size());
        const size_t encoded_len = encodeCOBS(
            unencoded[i].data(), 
            unencoded[i].size(), 
            encoded_output.data());

        REQUIRE(encoded_len == encoded[i].size());
        REQUIRE(encoded_output == encoded[i]);

        std::vector<uint8_t> decoded_output(unencoded[i].size());
        const size_t decoded_len = decodeCOBS(
            encoded_output.data(),
            encoded_output.size(),
            decoded_output.data()
        );

        REQUIRE(decoded_len == unencoded[i].size());
        REQUIRE(decoded_output == unencoded[i]);
    }
}

TEST_CASE("COBS handles 254-byte and 255-byte blocks", "[comms][cobs][edge]")
{
    SECTION("254 consecutive non-zero bytes")
    {
        std::vector<uint8_t> input(254);
        for (size_t i = 0; i < input.size(); ++i) {
            input[i] = static_cast<uint8_t>(i + 1);
        }

        std::vector<uint8_t> encoded(256); // !writing to data() doesnt grow the vector!!!!!
        const size_t encoded_len = encodeCOBS(input.data(), input.size(), encoded.data());

        REQUIRE(encoded_len == 256);
        REQUIRE(encoded[0] == 0xFF);
        REQUIRE(encoded.back() == 0x00);

        std::vector<uint8_t> decoded(input.size());
        const size_t decoded_len = decodeCOBS(encoded.data(), encoded.size(), decoded.data());
        REQUIRE(decoded_len == input.size());
        REQUIRE(decoded == input);
    }

    SECTION("255 bytes beginning with zero")
    {
        std::vector<uint8_t> input(255);
        input[0] = 0x00;
        for (size_t i = 1; i < input.size(); ++i) {
            input[i] = static_cast<uint8_t>(i);
        }

        std::vector<uint8_t> encoded(input.size() + 2); 
        const size_t encoded_len = encodeCOBS(input.data(), input.size(), encoded.data());

        REQUIRE(encoded_len == 257);
        REQUIRE(encoded[0] == 0x01);
        REQUIRE(encoded[1] == 0xFF);
        REQUIRE(encoded.back() == 0x00);

        std::vector<uint8_t> decoded(input.size());
        const size_t decoded_len = decodeCOBS(encoded.data(), encoded.size(), decoded.data());
        REQUIRE(decoded_len == input.size());
        REQUIRE(decoded == input);
    }
}
TEST_CASE("payload survives the UART round trip", "[comms][uart]") 
{
    use_test_device_id();

    msg_t in = make_msg(MSG_TYPE__SET_LED_PWM);
    in.payload.led_panel_cmd.led_panel_index = 12u;
    in.payload.led_panel_cmd.panel_percent   = 88.25f;

    encoded_msg_t enc{};
    REQUIRE(encode(&in, &enc));

    uint8_t tx_buffer[64]{};
    const size_t tx_len = convert_to_uart(&enc, enc.payload_len, tx_buffer);
    REQUIRE(tx_len > 0);
    REQUIRE(tx_len <= COMMS_UART_MSG_SIZE);

    encoded_msg_t back{};
    REQUIRE(convert_from_uart(tx_buffer, tx_len, &back));
    REQUIRE(back.id == enc.id);
    REQUIRE(back.payload_len == enc.payload_len);

    msg_t out{};
    REQUIRE(decode(&back, &out));
    REQUIRE(out.type == MSG_TYPE__SET_LED_PWM);
    REQUIRE(out.sender_id == kTestDeviceId);
    REQUIRE(out.payload.led_panel_cmd.led_panel_index == 12u);
    REQUIRE_THAT(out.payload.led_panel_cmd.panel_percent, WithinRel(88.25f));
}

TEST_CASE("corrupted payload gets 2016 game 7 LBJ'd", "[comms][uart]") 
{
    use_test_device_id();

    msg_t in = make_msg(MSG_TYPE__SET_LED_PWM);
    in.payload.led_panel_cmd.led_panel_index = 12u;
    in.payload.led_panel_cmd.panel_percent = 88.25f;

    encoded_msg_t enc{};
    REQUIRE(encode(&in, &enc));

    uint8_t frame[64]{};
    const size_t frame_len = convert_to_uart(&enc, enc.payload_len, frame);
    REQUIRE(frame_len > 0);

    encoded_msg_t received{};
    REQUIRE(convert_from_uart(frame, frame_len, &received));

    // Keep the checksum intact and re-encode so the COBS framing stays valid.
    uint8_t raw[64]{};
    const size_t raw_len = decodeCOBS(frame, frame_len, raw);
    REQUIRE(raw_len > 2);
    raw[raw_len - 3] ^= 0x01u;

    uint8_t corrupted[64]{};
    const size_t corrupted_len = encodeCOBS(raw, raw_len, corrupted);
    REQUIRE(corrupted_len > 0);
    REQUIRE_FALSE(convert_from_uart(corrupted, corrupted_len, &received));
}
