// Application
#pragma once
#include <usr_led.hpp>
#include <servo_interface.hpp>
#include <app_substates.hpp>
#include <can_interface.h>
#include <comms.h>

//TODO organize
#define this_device_id 0x00AC //Hex 0x1FFF is the max (13 bit)
static_assert((this_device_id) <= 0x1FFFu, "this_device_id exceeds 13-bit range (max 0x1FFF, min 0x10)");
static_assert((this_device_id) > 0x000F, "this_device_id is invalid. 0x0000 to 0x000F are reserved");


#define HEARTBEAT_PERIOD_MS 1000u
#define SENSOR_UPDATE_PERIOD_MS 1000u

enum class AppState 
{
    INIT,
    FANCY_STARTUP,
    IDLE,



    // Other random states.
    TEST_PANELS
};

enum class AppStatus
{
    OK,
    NOT_OK,
    CAN_INIT_ERROR,
    CAN_SEND_ERROR
};

//TODO LedStackController relies on hardware... improper interface
class App {
public:    
    App(ClockInterface& clock, ServoInterface& servo_tilt, ServoInterface& servo_pan, CanInterface& can)
    : substate_timer(Timer(clock, false)), heartbeat_timer(Timer(clock, true)), sensor_read_and_send_timer(Timer(clock, true)), clock(clock), servo_tilt(servo_tilt), servo_pan(servo_pan), can(can) {};

    void main(void);
private:
    Timer substate_timer;
    Timer heartbeat_timer;
    Timer sensor_read_and_send_timer;
    ClockInterface& clock;
    ServoInterface& servo_tilt;
    ServoInterface& servo_pan;
    CanInterface& can;

    AppState current_state;
    uint32_t current_substate;

    bool go_to_state(AppState next_state);
    void handle_app_error(AppStatus error_or_status, msg_t error_msg = {});

    //Communication helpers
    void send_msg(const msg_t &msg);
    void send_ping();
    void handle_heartbeat(); // Will send heartbeat at correct intervals
    void poll_can();
    // void handle_panel_cmd(LedPanelCMD_t& cmd);
    
    void handle_sending_light_readings();
    
    void run_state_machine(void);
    // State based functions. run_<state_name>
    // Not all states have a run function if they are simple enough
    void run_INIT(void);
    void run_FANCY_STARTUP(void);
    void run_TEST_PANELS(void);
        uint32_t test_panel_index = 0;
        float    test_duty        = 0.0f;
        bool     test_ramp_rising = true;

    // Once a state is complete, it runs this function.
    // This is what will switch current state
    void complete_state(AppStatus state_end_status);



    

};