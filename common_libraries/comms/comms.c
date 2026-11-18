#include "comms.h"
#include "crc.h"
//TODO tests tests tests
// Message Structure (not can specific, and not super efficient)
/*
Byte 0-1:    (2* bytes) Opener (Controller / device ID)  (* 13 usable bits + 3 padding bits))  
Byte 2-3:    (2 bytes)  Message Type ID
Byte 3-10:   (8 bytes)  Payload (data)

When transcoding for can, bytes 0 - 3 are contained within the can address
*/


/*
HOW TO ADD A MESSAGE 
* (Follow green comments)
IN THE HEADER FILE: (comms.h)
* Define a new message typedef. This defines the structure of your messsage. For example: "MyNewFeedback"
* Append the message type to the union (Payload_t)
* Append a new enum entry to the (msg_type_t) enum. Follow the naming pattern: "MSG_TYPE__MY_NEW_FEEDBACK"
IN THIS FILE: (comms.c)
* Add a case to expected_payload_len()
* Add a case to pack_payload(), using the PACK macro (just follow the pattern)
* Add a case to unpack_payload(), using the UNPACK macro (follow the pattern)
- Then make a pull request as soon as possible, or deal with minor merge conflicts (these PRs can just be "added message type")

Then, to use messages:
First, set your device id: comms_set_device_id(your_id) -- See device_ids.h

SENDING:
    msg_t msg;
    msg.type = MSG_TYPE__YOUR_NEW_MESSAGE;
    ! Do not touch msg.sender_id, it is filled automatically !
    msg.payload.your_message_type.data1 = ...; then fill in your data
    encoded_msg_t encoded_msg;
    encode(&msg, &encoded_msg); in -> out (sorry if you are really used to memcpy)
    Then, if sending over can:
    can_msg_t can_msg;
    convert_to_can(&encoded_msg, &can_msg);
    Then, use a can transport derrived from can_interface.h

RECEIVING;
    If from can:
    can_msg_t can_msg;
    Use a can transport derrived from can_interface.h
    encoded_msg_t encoded_msg;
    convert_from_can(&can_msg, &encoded_msg);
    msg_t msg;
    decode(&encoded_msg, &msg);
    
    Then, its up to the user, but a switch case works great:
    switch(msg.type)
    {
    case YOUR_MESSAGE_TYPE:
        handle_your_message_type(msg.payload.message_type);
    }

    Also, you can use msg.sender_id if you need to know who sent the message.
*/

//? Note: There is no built in CRCs or other checksums. This library is meant to be used with
//?     transport layers that handle checksums automagically (CAN, TCP)

// If this was CPP then we'd add a namespace {}.

// Why the heck are these do while(0)? - Its a standard way of using a macro that should be followed by a semicolon
#define PACK(buf, pos, x) \
    do { memcpy((buf) + (pos), &(x), sizeof(x)); (pos) += sizeof(x); } while (0)
 
#define UNPACK(buf, pos, x) \
    do { memcpy(&(x), (buf) + (pos), sizeof(x)); (pos) += sizeof(x); } while (0)
 
static uint16_t s_device_id = COMMS_INVALID_DEVICE_ID;
 
void comms_set_device_id(uint16_t device_id) { s_device_id = device_id & COMMS_DEVICE_ID_MASK; }
uint16_t comms_get_device_id(void)           { return s_device_id; }
 
// ---- id pack/unpack ---------------------------------------------------------
uint32_t pack_id(uint16_t device_id, uint16_t type)
{
    return (((uint32_t)(device_id & COMMS_DEVICE_ID_MASK)) << COMMS_DEVICE_ID_SHIFT)
         | ((uint32_t)type & COMMS_MSG_ID_MASK);
}
uint16_t id_device(uint32_t id)
{
    return (uint16_t)((id >> COMMS_DEVICE_ID_SHIFT) & COMMS_DEVICE_ID_MASK);
}
uint16_t id_type(uint32_t id)
{
    return (uint16_t)(id & COMMS_MSG_ID_MASK);
}

// The raw frame is fixed-width and carries no length field, so the receiver
// must know how many payload bytes a given type occupies.
// Using sizeof() on structs will often return the wrong value (structs are padded.)
static size_t expected_payload_len(uint16_t type)
{
    switch (type) {
        case MSG_TYPE__PING:                                return 2u * sizeof(uint32_t);           // 8
        case MSG_TYPE__HEARTBEAT:                           return 4u;
        case MSG_TYPE__SET_LED_PWM:                         return sizeof(uint8_t) + sizeof(float); // 5
        case MSG_TYPE__HARDWARE_ERROR:                      return 2u;
        case MSG_TYPE__LED_PANEL_FEEDBACK:                  return sizeof(uint8_t) + sizeof(float);
        case MSG_TYPE__PTZ_VEL_CMD:                         return 8u;
        case MSG_TYPE__PTZ_FEEDBACK:                        return 8u;
        case MSG_TYPE__MINI_ROVER_VEL_CMD:                  return 8u;

        //* ADD YOUR MESSAGE ABOVE THIS LINE, RETURN THE SIZE OF IT
        default:                                            return SIZE_MAX; 
    }
}

// Messages that are not meant to go over can.
// Set explicitly so user doesn't accedentally make a CAN message that is too big
// Could also be used to distinguish against CAN and CANFD messages, but if you are only using
//      this comms library on CAN, you can ignore this.
static bool is_large_message(uint16_t type)
{
    // No large messages rn.
    switch (type) {
        default:                                                return false;
    }
}
 
// ---- payload (de)serialization (field-by-field, padding-safe) ---------------
// returns bytes written, or SIZE_MAX on error
static size_t pack_payload(const msg_t* msg, uint8_t* out, size_t cap)
{
    size_t pos = 0;
    switch (msg->type) {
    case MSG_TYPE__PING:
        PACK(out, pos, msg->payload.ping.reserved_1);
        PACK(out, pos, msg->payload.ping.reserved_2);
        break;
    case MSG_TYPE__HEARTBEAT:
        PACK(out, pos, msg->payload.heartbeat.timestamp_ms);
        break;
    case MSG_TYPE__SET_LED_PWM: {
        float pct = msg->payload.led_panel_cmd.panel_percent;
        // if (pct < 0.0f || pct > 100.0f) return SIZE_MAX;        // range guard if we want it?
        PACK(out, pos, msg->payload.led_panel_cmd.led_panel_index);
        PACK(out, pos, msg->payload.led_panel_cmd.panel_percent);
        break;
    }
    case MSG_TYPE__LED_PANEL_FEEDBACK:
        PACK(out, pos, msg->payload.led_panel_feedback.led_panel_index);
        PACK(out, pos, msg->payload.led_panel_feedback.panel_percent);
        break;
    case MSG_TYPE__HARDWARE_ERROR:
        PACK(out, pos, msg->payload.hardware_error.error_type);
        PACK(out, pos, msg->payload.hardware_error.info);
        break;
    case MSG_TYPE__PTZ_VEL_CMD:
        PACK(out, pos, msg->payload.ptz_vel_cmd.vel_tilt);
        PACK(out, pos, msg->payload.ptz_vel_cmd.vel_pan);
        break;
    case MSG_TYPE__PTZ_FEEDBACK:
        PACK(out, pos, msg->payload.ptz_feedback.curr_vel_tilt);
        PACK(out, pos, msg->payload.ptz_feedback.curr_vel_pan);
        break;
    case MSG_TYPE__MINI_ROVER_VEL_CMD:
        PACK(out, pos, msg->payload.mini_rover_velcmd.lin_vel_mps);
        PACK(out, pos, msg->payload.mini_rover_velcmd.ang_vel_degps);
        break;

    //* ADD YOUR MESSAGE TYPE HERE, USING THE PACK MACRO
    default:
        return SIZE_MAX; // This is an error
    }
    return pos;
}
 
static bool unpack_payload(msg_t* msg, const uint8_t* in, size_t len)
{
    size_t pos = 0;
    switch (msg->type) {
    case MSG_TYPE__PING:
        UNPACK(in, pos, msg->payload.ping.reserved_1);
        UNPACK(in, pos, msg->payload.ping.reserved_2);
        break;
    case MSG_TYPE__HEARTBEAT:
        UNPACK(in, pos, msg->payload.heartbeat.timestamp_ms);
        break;
    case MSG_TYPE__SET_LED_PWM:
        UNPACK(in, pos, msg->payload.led_panel_cmd.led_panel_index);
        UNPACK(in, pos, msg->payload.led_panel_cmd.panel_percent);
        break;
    case MSG_TYPE__LED_PANEL_FEEDBACK:
        UNPACK(in, pos, msg->payload.led_panel_feedback.led_panel_index);
        UNPACK(in, pos, msg->payload.led_panel_feedback.panel_percent);
        break;
    case MSG_TYPE__HARDWARE_ERROR:
        UNPACK(in, pos, msg->payload.hardware_error.error_type);
        UNPACK(in, pos, msg->payload.hardware_error.info);
        break;
    case MSG_TYPE__PTZ_VEL_CMD:
        UNPACK(in, pos, msg->payload.ptz_vel_cmd.vel_tilt);
        UNPACK(in, pos, msg->payload.ptz_vel_cmd.vel_pan);
        break;
    case MSG_TYPE__PTZ_FEEDBACK:
        UNPACK(in, pos, msg->payload.ptz_feedback.curr_vel_tilt);
        UNPACK(in, pos, msg->payload.ptz_feedback.curr_vel_pan);
        break;
    case MSG_TYPE__MINI_ROVER_VEL_CMD:
        UNPACK(in, pos, msg->payload.mini_rover_velcmd.lin_vel_mps);
        UNPACK(in, pos, msg->payload.mini_rover_velcmd.ang_vel_degps);
        break;

    //* ADD YOUR MESSAGE TYPE HERE, USING THE UNPACK MACRO
    default:
        return false;
    }
    (void)pos;
    return true;
}

// User functions
bool encode(const msg_t* msg_in, encoded_msg_t* encoded_msg_out)
{
    if (!msg_in || !encoded_msg_out) return false;
    if(s_device_id == COMMS_INVALID_DEVICE_ID) return false;
 
    memset(encoded_msg_out->payload, 0, sizeof encoded_msg_out->payload);
    size_t n = pack_payload(msg_in, encoded_msg_out->payload,
                            sizeof encoded_msg_out->payload);
    if (n == SIZE_MAX) return false;
 
    encoded_msg_out->id  = pack_id(s_device_id, msg_in->type);
    encoded_msg_out->payload_len = (uint8_t)n;
    return true;
}
 
bool decode(const encoded_msg_t* encoded_msg_in, msg_t* msg_out)
{
    if (!encoded_msg_in || !msg_out) return false;
    if (encoded_msg_in->payload_len > MAX_LARGE_MESSAGE_PAYLOAD_SIZE_BYTES) return false;
 
    msg_out->type = id_type(encoded_msg_in->id);
    msg_out->sender_id = id_device(encoded_msg_in->id); // Preserve device ID, so we know which device sent this message

    return unpack_payload(msg_out, encoded_msg_in->payload, encoded_msg_in->payload_len);
}
 
bool convert_to_can(const encoded_msg_t* in, can_msg_t* out)
{
    if (!in || !out) return false;
    if (in->payload_len > MAX_MESSAGE_PAYLOAD_SIZE_BYTES) return false;
 
    out->id  = in->id & COMMS_CAN_EXT_ID_MASK;   // 29-bit extended id
    out->payload_len = in->payload_len;                          // DLC = actual payload payload_length
    memcpy(out->payload, in->payload, in->payload_len);
    return true;
}
 
bool convert_from_can(const can_msg_t* in, encoded_msg_t* out)
{
    if (!in || !out) return false;
    if (in->payload_len > MAX_MESSAGE_PAYLOAD_SIZE_BYTES) return false;
 
    out->id  = in->id & COMMS_CAN_EXT_ID_MASK;
    out->payload_len = in->payload_len;
    memset(out->payload, 0, sizeof out->payload);
    memcpy(out->payload, in->payload, in->payload_len);
    return true;
}
 
bool convert_to_raw(const encoded_msg_t* in, uint8_t* raw_out)
{
    if (!in || !raw_out) return false;
    
    size_t   pos  = 0;
    uint16_t dev  = id_device(in->id);
    uint16_t type = id_type(in->id);
    
    size_t max_payload_size = is_large_message(type) ? MAX_LARGE_MESSAGE_PAYLOAD_SIZE_BYTES : MAX_MESSAGE_PAYLOAD_SIZE_BYTES;
    if (in->payload_len > max_payload_size) return false;
 
    PACK(raw_out, pos, dev);    // bytes 0-1: device id (LE)
    PACK(raw_out, pos, type);   // bytes 2-3: message id (LE)
    // bytes 4-11: payload, zero-padded to the fixed frame width (unless its large message)
    memset(raw_out + pos, 0, max_payload_size);
    memcpy(raw_out + pos, in->payload, in->payload_len);
    return true;
}
 
bool convert_from_raw(const uint8_t* raw_in, encoded_msg_t* out)
{
    if (!raw_in || !out) return false;
 
    size_t   pos  = 0;
    uint16_t dev  = 0;
    uint16_t type = 0;
 
    UNPACK(raw_in, pos, dev);
    UNPACK(raw_in, pos, type);
 
    size_t plen = expected_payload_len(type);    // raw frame carries no length
    if (plen == SIZE_MAX) return false;           // unknown type
 
    out->id  = pack_id(dev, type);
    out->payload_len = (uint8_t)plen;
    memset(out->payload, 0, sizeof out->payload);
    memcpy(out->payload, raw_in + pos, plen);
    return true;
}

/*---------------------- UART-Specific Stuff -------------------------------- */
/* 
 * COBS (Consistent Overhead Byte Stuffing) is well-established framing thingy for UART, using 0x00 as delimiter 
 ? COBS implementation based on https://en.wikipedia.org/wiki/Consistent_Overhead_Byte_Stuffing#Implementation
 TODO: figure out if memcpy is better 
*/
size_t encodeCOBS(const void *data, size_t length, uint8_t *buffer) {
    // assert(data && buffer); // originally this, idk why 
    if (!data || !buffer) return 0;

    uint8_t *encode = buffer; // encoded byte pointer 
    uint8_t *codep = encode++; // Output code pointer
    uint8_t code = 1; // Code value 

    for (const uint8_t *byte = (const uint8_t *)data; length--; ++byte) { // wtf 
        if (*byte) // byte not zero, write it
            *encode++ = *byte, ++code; 
        if (!*byte || code == 0xff) {
            *codep = code, code = 1, codep = encode;
            if (!*byte || length)
                ++encode;
        }
    }
    *codep = code; // write final code value 
    *encode++ = 0x00; // append delimiter 
    return (size_t)(encode - buffer);
}

size_t decodeCOBS(const uint8_t *buffer, size_t length, void *data) {
    if (!data || !buffer) return 0; // dont do anything in the case for null ptr 
    
    const uint8_t *byte = buffer; // encoded input byte pointer 
    uint8_t *decode = (uint8_t *)data; // decoded output byte buffer 

    for (uint8_t code = 0xff, block = 0; byte < buffer + length; --block)  // dafuq why is there so many params 
    {
        if (block) // decode block byte, wtf is block?
            *decode++ = *byte++;
        else {
            block = *byte++;
            if (block && (code != 0xff))
                *decode++ = 0;
            code = block;
            if (!code) // delimiter found
                break;
        }
    }
    return (size_t)(decode - (uint8_t *)data);
}

// TODO implement convert_to_can and convert_from_uart
// [COBS overhead byte][raw message encoded][CRC16][0x00 delimiter] = 1 + 14 + 
size_t convert_to_uart(const encoded_msg_t *inptr, size_t len, uint8_t *outptr) {
    if (!inptr || !outptr) return false; // reject nullptr's 
    uint8_t frame_buf[COMMS_UART_MSG_SIZE]; // issa fixed length 
    if (!convert_to_raw(inptr, frame_buf)) return false; 
    
    uint16_t crc = crc16(frame_buf, COMMS_RAW_MSG_SIZE);
    size_t pos = COMMS_RAW_MSG_SIZE; // PACK macro updates pos here 
    PACK(frame_buf, pos, crc); // PACK macro packs stuff little-endianly 

    size_t encoded_len = encodeCOBS(frame_buf, pos, outptr);
    return encoded_len;
}

bool convert_from_uart(const uint8_t *inptr, size_t len, encoded_msg_t *outptr) {
    if (!inptr || !outptr || len < COMMS_UART_MSG_SIZE) return false;

    // Decode COBS frame (16 bytes -> 14 bytes: 12-byte raw frame + 2-byte CRC16)
    uint8_t raw_with_crc[COMMS_RAW_MSG_SIZE + COMMS_CRC16_SIZE];
    size_t decoded_len = decodeCOBS(inptr, len, raw_with_crc);
    if (decoded_len != sizeof(raw_with_crc)) return false;

    // Unpack the received little-endian CRC16 from bytes 12-13
    uint16_t rx_crc = 0;
    size_t pos = COMMS_RAW_MSG_SIZE;
    UNPACK(raw_with_crc, pos, rx_crc);

    // Recompute CRC over bytes 0-11 and verify
    uint16_t calc_crc = crc16(raw_with_crc, COMMS_RAW_MSG_SIZE);
    if (rx_crc != calc_crc) return false;

    // Convert the verified 12-byte raw frame into encoded_msg_t
    return convert_from_raw(raw_with_crc, outptr);
}