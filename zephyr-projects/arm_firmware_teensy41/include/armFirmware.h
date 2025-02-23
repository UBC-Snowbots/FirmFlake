/*
Created By: Rowan Z
Created On: August 17, 2023
Updated On: 
Description: Header file for firmware for driving a 6 axis arm via ROS on a teensy 4.1 MCU
*/

//TODO, 
/* more tags throughout code in this zephyr project

crashes with more than 3 axis -- wrong gpio, pin 11 was messed
weird shit in the buffer, but will just sanitize -- not processing tx buffer data properly



this is not easy

*/
#pragma once
#include <stdlib.h>
#include <stdio.h>

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/uart.h>
#include <zephyr/sys/ring_buffer.h>
#include <zephyr/logging/log.h>


//usb (needed for uart)
#include <zephyr/usb/usb_device.h>
#include <zephyr/usb/usbd.h>

#include <zephyr/drivers/gpio.h>

#include <Axis.h>

#include <string>

// #define UART_DEVICE_NODE DT_CHOSEN(zephyr_shell_uart)
//#define UART_DEVICE DT_NODELABEL(lpuart4) 
// #define UART_DEV        DT_NODELABEL(lpuart4)
#define MSG_SIZE 32

#define DEBUG_MSGS true

// general parameters
#define NUM_AXES 7
#define ON 0
#define OFF 1
#define SW_ON 0
#define SW_OFF 1
#define FWD 1
#define REV 0

#define RING_BUF_SIZE 2048
#define RX_BUF_SIZE 128
#define TX_BUF_SIZE 128

#define ARM_DEG_RESOLUTION 0.01
#define ARM_DEG_VELOCITY_RESOLUTION 0.05
#define EE_INDEX 6

#define HOME 1
#define ABSOLUTE_TARGET_POSITION 2
#define INCREMENTAL_TARGET_POSITION 3
#define TARGET_VELOCITY_ID 4 //sets the target velocity, but doesnt command arm, will only move if there are steps_remaining
#define TEST_LIMITS 5
#define ABSOLUTE_TARGET_VELOCITY_ID 6 //sets target velocity, and commands arm to move at that velocity
#define HELP_CMDTYPE 7


#define POSITION_CONTROL 1 
#define VELOCITY_CONTROL 2 
#define PRO_MODE false //removes accelleration limits. This is dangerous, but gives the operator full control over when the arm stops


#define NUM_PRESET_POSITIONS 1
#define DEFAULT_POSITION 0

#define POSITION_PING_MS_INTERVAL 10 // 50ms is 20 hz // 20ms is 50hz // 10ms is 100hz 

#define SPEED_PING_MS_INTERVAL 150 // 50ms is 20 hz


#define ARM_COMM_RATE 20 //Hz, only here for refrence

//comm characters
#define HOME_CHAR 'h'
#define ANGLE_CALLBACK_CHAR 'A'
#define HELP_CHAR '?'

#define ACCEL_CURVE 1





//weird pin mappings
//all the usefull documentation I could find: https://docs.zephyrproject.org/latest/boards/arm/teensy4/doc/index.html 
// https://docs.zephyrproject.org/latest/boards/pjrc/teensy4/doc/index.html

// Motor pins     
// inline int stepPins[6] =   {6, 8, 2, 10, 12, 25}; 
// inline int dirPins[6] =    {5, 7, 1, 9, 11, 24}; 

//To GPIO devs                  //not working: 3 //working one dir: 1   // working: 2, 4, 5, 6 
inline int stepGPIO_PIN[NUM_AXES][2] =   {{2,10}, {2,16}, {1,3}, {2,0}, {2,1}, {1,13}, {2,18}}; 
inline int dirGPIO_PIN[NUM_AXES][2] =    {{1,30}, {2,17}, {1,2}, {2,11}, {2,2}, {1,12}, {2,19}}; 
// inline int stepGPIO_PIN[NUM_AXES][2] =   {{2,10}, {2,16}, {1,3}, {2,0}, {2,1}, {2,18}, {1, 13}}; 
// inline int dirGPIO_PIN[NUM_AXES][2] =    {{1,30}, {2,17}, {1,2}, {2,11}, {2,2}, {2,19}, {1, 12}}; 
inline int arpo = 0;

// Encoder pins
// inline int encPinA[6] = {17, 38, 40, 36, 13, 15};
// inline int encPinB[6] = {16, 37, 39, 35, 41, 14};

// limit switch pins
// inline int limPins[6] = {18, 19, 20, 21, 23, 22};
inline int limGPIO_PIN[NUM_AXES][2] = {{1,17}, {1,16}, {1,26}, {1,27}, {1,25}, {1,24}, {4, 8}};

// pulses per revolution for motors
inline int ppr[NUM_AXES] = {400, 400, 400, 400, 400, 400, 400};

// Gear Reductions
// inline float red[6] = {50.0, 160.0, 92.3077, 43.936, 57.0, 5.18}; 
inline float red[NUM_AXES] = {50.0, 160.0, 92.3077, 250, 57.0, 170.00, 100.0}; //? Double check with arm 
// inline float red[6] = {50.0, 160.0, 92.3077, 43.936, 57.0, 20.00}; //? Double check with arm -> August 11th
//?A6 10:17 for belts -> 170 to one

//from old driver
// // Motor pins      
// int stepPins[6] =   {6, 8, 2, 10, 12, 25}; 
// int dirPins[6] =    {5, 7, 1, 9, 11, 24}; 

// // Encoder pins
// int encPinA[6] = {17, 38, 40, 36, 13, 15};
// int encPinB[6] = {16, 37, 39, 35, 41, 14};

// // limit switch pins
// int limPins[6] = {18, 19, 20, 21, 23, 22};

// // pulses per revolution for motors
// long ppr[6] = {400, 400, 400, 400, 400, 400};

// // Gear Reductions
// float red[6] = {50.0, 160.0, 92.3077, 43.936, 57.0, 5.18};

// End effector variables
const int closePos = 0; 
const int openPos = 9000; //TODO: needs to be set with proper step value for gear redcution and ppr
const int EEstepPin = 4;
const int EEdirPin = 3;
const int speedEE = 1000;
const int accEE = 1000;
const int MOTOR_DIR_EE = -1;
const int forcePin = 12;


inline bool arm_homing = false;
inline bool arm_inited = false;


// Encoder Variables
inline int curEncSteps[NUM_AXES], cmdEncSteps[NUM_AXES];
const int pprEnc = 512;
// const float ENC_MULT[] = {5.12, 5.12, 5.12, 5.12, 5.12, 5.12};
inline float ENC_STEPS_PER_DEG[NUM_AXES];

inline int control_mode = VELOCITY_CONTROL;


//this creates the device for gpio pins
inline const struct device *gpio1_dev;
inline const struct device *gpio2_dev;
inline const struct device *gpio3_dev;
inline const struct device *gpio4_dev;


inline struct ring_buf ringbuf;


inline const struct device *dev;
//inline char tx_msg[TX_BUF_SIZE];

void sendMsg(const char tx_msg[TX_BUF_SIZE]);

//timer callback to handle step signals
void stepper_timer_callback(struct k_timer *timer_id);

void home_timer_callback(struct k_timer *timer_id);
void accelTimer_callback(struct k_timer *timer_id);


void limit_switch_callback(const struct device *dev, struct gpio_callback *cb, uint32_t pins);

void parseCmd(uint8_t cmd[RX_BUF_SIZE]);
void parseAbsoluteTargetPositionCmd(uint8_t cmd[RX_BUF_SIZE]);
void parseIncrementalTargetPositionCmd(uint8_t cmd[RX_BUF_SIZE]);
void parseHomeCmd(uint8_t cmd[RX_BUF_SIZE]);
void parseHelpCmd(uint8_t cmd[RX_BUF_SIZE]);
void parseSettingCmd(uint8_t cmd[RX_BUF_SIZE]);
void parseAbsoluteTargetVelocityCmd(uint8_t cmd[RX_BUF_SIZE]);

void testLimits();


void initilizeAxis(struct InstAxis *instance);
void stepAxis(int axis);
void stepAll_timer_callback(struct k_timer *timer_id);
void pingStepPosition_timer_callback(struct k_timer *timer_id);
void pingAnglePosition_timer_callback(struct k_timer *timer_id);

int angleToSteps(float angle, int i);
float stepsToAngle(int steps, int i);
int degPerSecToUsecPerStep(float angle, int i);
int degPerSecToMsecPerStep(float angle, int i);
float usecPerStepToDegPerSec(int steps, int i);


void updateAngles();

//void stepTo(int pos);
void homeAllAxes();
//inline bool homed[NUM_AXES] = {0};

void goto_preset(int position);

int pos_or_negative_float(float val);


void set_gpio(int dev, int pin, int value);
int get_gpio(int dev, int pin);




