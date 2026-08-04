#pragma once

#ifdef __cplusplus
extern "C" {
#endif
#include "device_ids.h"
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>
#include <assert.h>   // static_assert (C11). Compile with -std=c11 or later.
// To be included by each device

static_assert(sizeof(float) == 4, "Comms requires 32-bit floats");

//* Using CAN
#define MAX_PAYLOAD_SIZE_BYTES_CAN 8u
// #define MAX_PAYLOAD_SIZE_BYTES_CANFD 64u

#define MAX_MESSAGE_PAYLOAD_SIZE_BYTES MAX_PAYLOAD_SIZE_BYTES_CAN
#define MAX_LARGE_MESSAGE_PAYLOAD_SIZE_BYTES 264u //Probably ignore if using regular CAN

// Don't touch these
#define COMMS_HEADER_SIZE_BYTES 4u
#define COMMS_RAW_MSG_SIZE      (COMMS_HEADER_SIZE_BYTES + MAX_MESSAGE_PAYLOAD_SIZE_BYTES)
#define COMMS_DEVICE_ID_MASK   0x1FFFu      // Mask to 13 bits
#define COMMS_DEVICE_ID_SHIFT  16u
#define COMMS_MSG_ID_MASK      0xFFFFu      // Mask to 16 bits
#define COMMS_CAN_EXT_ID_MASK  0x1FFFFFFFu  // Mask to 29 bits
#define COMMS_INVALID_DEVICE_ID 0x1FFFu     // Set as default. 


#define PING_RESERVED_1 0xBEEFu //idk
#define PING_RESERVED_2 0xABBAu //idk

// If you need an enum inside your message (oppossed to adding lots of message types)
// You can add them here.
enum HardwareErrorType {
    HARDWARE_ERROR_TYPE__UNKNOWN,
    HARDWARE_ERROR_TYPE__CONFIGURATION_ERROR, // Software can be changed to fix the problem, without needing to reflash firmware
    HARDWARE_ERROR_TYPE__INCORRECT_FIRMWARE_VERSION, // Firmware must be re flashed to fix the problem (some board is out of date)
    HARDWARE_ERROR_TYPE__PHYSICALLY_FUCKED, // Something physical broke (wires severed, part overheated) Usually hard to detect
};

 
// Message structures
// Every payload must be no more than max payload size (currently 8 bytes)

typedef struct {
    uint32_t reserved_1;
    uint32_t reserved_2;
} Ping_t;

// Similar to ping
typedef struct {
    uint32_t timestamp_ms;
} HeartBeat_t;

typedef struct {
    uint8_t led_panel_index;
    float panel_percent;
    bool is_response; // A response is the LED controller responding with the currently set value
} LedPanelCMD_t;

typedef struct {
    uint8_t led_panel_index;
    float panel_percent;
} LedPanelFeedback_t;

typedef struct {
    uint8_t error_type;
    uint8_t info;
} HardwareError_t;

typedef struct {
    float vel_tilt;
    float vel_pan;
    // Thats all we got rn, open loop control
} PTZVelCmd_t;

typedef struct {
    float curr_vel_tilt;
    float curr_vel_pan;
    // Thats all we got rn, open loop control
} PTZFeedback_t;

typedef union {
    Ping_t          ping;
    HeartBeat_t     heartbeat;
    LedPanelCMD_t   led_panel_cmd;
    LedPanelFeedback_t led_panel_feedback;
    HardwareError_t     hardware_error;
    PTZVelCmd_t         ptz_vel_cmd;
    PTZFeedback_t       ptz_feedback;
} Payload_t;

// C friendly comms helper functions
typedef struct msg_t
{
    uint16_t sender_id; // Only used when decoding messages, ignored when encoding
    uint16_t type;
    Payload_t payload;
} msg_t;

// Why are encoded msg and can msg two different structs? -> in case we want to have big encoded messages and send them over multile can messages
// Currently, they are the same.
typedef struct encoded_msg_t
{
    uint32_t id;
    uint8_t  payload_len;
    uint8_t payload[MAX_MESSAGE_PAYLOAD_SIZE_BYTES];
} encoded_msg_t;

typedef struct can_msg_t
{
    uint32_t id; // extended IDs
    uint8_t payload_len; // length of payload (0 ... MAX_MESSAGE_PAYLOAD_SIZE_BYTES)
    uint8_t payload[MAX_MESSAGE_PAYLOAD_SIZE_BYTES];
} can_msg_t;

typedef enum {
    MSG_TYPE__UNKNOWN, // 0 INDEX IS RESERVED, DO NOT USE
    MSG_TYPE__PING, // Just to ping a device. If the device has any lights, maybe this would make them flash
    MSG_TYPE__HEARTBEAT,
    MSG_TYPE__SET_LED_PWM,
    MSG_TYPE__HARDWARE_ERROR, // Not really used rn
    MSG_TYPE__LED_PANEL_FEEDBACK,
    MSG_TYPE__PTZ_VEL_CMD,
    MSG_TYPE__PTZ_FEEDBACK,
    NUM_MSG_TYPES
} msg_type_t;

uint32_t pack_id(uint16_t device_id, uint16_t type);
uint16_t id_device(uint32_t id);
uint16_t id_type(uint32_t id);

void     comms_set_device_id(uint16_t device_id);
uint16_t comms_get_device_id(void);

bool encode(const msg_t* msg_in, encoded_msg_t* encoded_msg_out);
bool decode(const encoded_msg_t* encoded_msg_in, msg_t* msg_out);

bool convert_to_can  (const encoded_msg_t* in, can_msg_t* out);
bool convert_from_can(const can_msg_t* in, encoded_msg_t* out);

bool convert_to_raw  (const encoded_msg_t* in, uint8_t* raw_out);
bool convert_from_raw(const uint8_t* raw_in, encoded_msg_t* out);
bool len_to_dlc(const uint8_t len, uint32_t* dlc_out);
bool dlc_to_len(const uint32_t dlc, uint8_t* len_out);



#ifdef __cplusplus
}
#endif
