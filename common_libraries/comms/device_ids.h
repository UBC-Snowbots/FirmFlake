#pragma once

// MAX DEVICE ID is 8191 decimal (max of 13 bit)
// 0x1FFF in hex

// To set your device ID,
// void comms_set_device_id(YOUR_DEVICE_ID) 

#define DEVICE_ID__LED_PANEL_BASE 0x0100
#define MAX_NUM_LED_PANELS 0xF // 0x0100 -> 0x010F are reserved for LED panels
// then, each individual panel gets an offset.
// OR just define each explicitly:
//#define DEVICE_ID__LED_PANEL_0 0x0100
//#define DEVICE_ID__LED_PANEL_1 0x0101
//#define DEVICE_ID__LED_PANEL_2 0x0102
// ...

// Will prob redefine these later
#define DEVICE_ID__ON_BOARD_NUC_NUBS 0x0C01
#define DEVICE_ID__CONTROL_BASE_SUC 0x0C02
#define DEVICE_ID__ON_BOARD_JETSON_JETSKI 0x0C03
