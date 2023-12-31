#pragma once
#include <armFirmware.h>
// #define INIT_AXIS(i) Axis(stepPins[i], dirPins[i], encPinA[i], encPinB[i], ppr[i], red[i])

//if an axis is within this many steps to its max/min
#define POSITION_STEP_LIMIT_THRESHOLD 50

struct Axis {

    //variables don't get properly set here, even if "int foo = 1000;" foo will still equal 0. initilize in the main function of main.cpp
   // Axis(int dirPin, int stepPin, int encPinA, int encPinB, long ppr, float reduction);
    //setup pins
    int index;
    int LIMIT_PIN[2];
    int DIR_PIN[2];
    int STEP_PIN[2];
    int ENC_PIN_A;
    int ENC_PIN_B;
    //motor gear reduction and driver PPR (make sure to factor in microstep settings)
    int PPR;
    float REDUCTION;
    bool pulse_state;
    bool moving;
    
    //units here are usec per step
    int max_speed;
    int home_speed;
    volatile int current_speed = 10000;
    volatile int target_speed = 10000;
    int max_start_speed = 20000;
    int zero_speed = 10000; //limits minimum speed to a feasible start speed


    int home_accel = 0;
    int start_accel = 250; 
    int accel_slope = 10; //should stay at 1 for maximum smoothness
    int current_accel = 20000;

    float target_velocity;
    float current_velocity;
    float max_velocity;
    float home_velocity;

    int macrostep;

    bool dir;
    bool last_dir = 0;
    //bool dir_signal;
    int home_dir;
    int home_dir_vel;
    bool homing;
    bool homed = false;
    int preset_step_pos[1];

    int max_step_pos;
    int min_step_pos = 20 + POSITION_STEP_LIMIT_THRESHOLD;

    volatile int steps_remaining;
    volatile int step_pos = 0; // not from encoders
    volatile int step_des_pos;

    int decel_min_steps;

    volatile float des_angle_pos = 0.0;
    volatile float angle_pos = 0.0;
    //float max_angle_
    
    struct k_timer stepper_timer;
    struct k_timer accel_timer;
    struct gpio_callback limit_switch_cb_data;

//   // Code to configure pins
//   const struct device *stepdev;
//   const struct device *dirdev;
//   const struct device *limdev;
//   static struct gpio_callback limit_switch_cb_data;


    void attach();
    void readEncoder();
    void updateAngles();

    //void step(int steps);
    void home();

 
    
};