#include <main.hpp>
#include <app.hpp>

// After setup, program enters here.
void App::main(void)
{
    current_state = AppState::INIT;
    // usr_leds.init();
    // usr_leds.turn_all_on();
    for(;;)
    {
        // poll comms?
        run_state_machine();
        handle_ptz_timeout();   // runs in every state, not just IDLE
        // usr_leds.update();
        handle_heartbeat(); // Send heartbeat at regular intervals
    }
}

void App::poll_can()
{
    can_msg_t can_msg = {};
    if(can.read(can_msg) != CanStatus::OK)
    {
        //TODO better error handling
        return;
    }
    // usr_leds.flash(UsrLedIndex::LED_2, 1, 10);


    encoded_msg_t encoded_msg = {};
    msg_t msg = {}; // decoded message
    convert_from_can(&can_msg, &encoded_msg);
    decode(&encoded_msg, &msg);

    switch(msg.type)
    {
        case MSG_TYPE__PING:
            // A ping came in from another device
            // usr_leds.flash(UsrLedIndex::LED_0);
            break;

        case MSG_TYPE__UNKNOWN:
            break;
        case MSG_TYPE__PTZ_VEL_CMD:
            // handle_ptz_vel_cmd(msg.payload)
            {
                servo_tilt.set_speed(msg.payload.ptz_vel_cmd.vel_tilt);
                servo_tilt.set_speed(msg.payload.ptz_vel_cmd.vel_pan);
                msg_t ptz_feedback_msg;
                ptz_feedback_msg.type = MSG_TYPE__PTZ_FEEDBACK;
                ptz_feedback_msg.payload.ptz_feedback.curr_vel_pan = msg.payload.ptz_vel_cmd.vel_pan;
                ptz_feedback_msg.payload.ptz_feedback.curr_vel_tilt = msg.payload.ptz_vel_cmd.vel_tilt;
                send_msg(ptz_feedback_msg);
                ptz_cmd_timer.set(PTZ_CMD_TIMEOUT_MS);                 // pet the watchdog
            }
            break;
    }
}

void App::handle_ptz_timeout(void)
{
    if(!ptz_cmd_timer.expired()) return;

    // Comms lost or commander stopped talking -> fail safe to stopped
    ptz_cmd_timer.disable();  // one-shot; re-armed by the next PTZ cmd
    servo_pan.set_speed(0.0f);
    servo_tilt.set_speed(0.0f);
}

// Run based on current state
void App::run_state_machine(void)
{
    using enum AppState;
    switch (current_state)
    {
    case INIT:
        run_INIT();
        break;
    case FANCY_STARTUP:
        // run_FANCY_STARTUP();
        break;
    case TEST_PANELS:
        // run_TEST_PANELS();
        break;
    case IDLE:
        poll_can(); // poll can on idle
        // handle_sending_light_readings();
        // servo_tilt.set_speed(-100.0);
        break;
    default:
        break;
    }
}

void App::complete_state(AppStatus state_end_status = AppStatus::OK)
{
    using enum AppState;
    // Reset current substate
    current_substate = NULL_STATE;

    if(state_end_status == AppStatus::OK)
    {
        // Happy Path
        switch (current_state)
        {
        case INIT:
            go_to_state(IDLE);
            break;
        case FANCY_STARTUP:
            // go_to_state(TEST_PANELS);
            go_to_state(IDLE);
            break;
        case TEST_PANELS:
            go_to_state(IDLE);
            break;
        case IDLE:
            // do nothing
            break;
            
            break;
        default:
            break;
        }
        return;
    }
}

bool App::go_to_state(AppState next_state)
{
    // Clear substate_timer
    substate_timer.disable();

    current_substate = NULL_STATE;
    
    // Update current state
    this->current_state = next_state;
    return true;
}

void App::run_INIT(void)
{

    // Set Device ID
    // comms_set_device_id(this_device_id);
    // expected_payload_len();
    // if(usr_leds.init() != UsrLedStatus::LED_OK)
    // {
    //     handle_app_error(AppStatus::NOT_OK);
    //     return;
    // }

    comms_set_device_id(this_device_id);

    if(can.init() != CanStatus::OK)
    {
        handle_app_error(AppStatus::CAN_INIT_ERROR);
        return;
    }
    
    if(substate_timer.init() != true)
    {
        handle_app_error(AppStatus::NOT_OK);
        return;
    }
    if(heartbeat_timer.init() != true)
    {
        handle_app_error(AppStatus::NOT_OK);
        return;
    }
    if(ptz_cmd_timer.init() != true)
    {
        handle_app_error(AppStatus::NOT_OK);
        return;
    }
    // if(sensor_read_and_send_timer.init() != true)
    // {
    //     handle_app_error(AppStatus::NOT_OK);
    //     return;
    // }
    heartbeat_timer.set(HEARTBEAT_PERIOD_MS);
    // sensor_read_and_send_timer.set(SENSOR_UPDATE_PERIOD_MS);

    servo_tilt.init();
    servo_pan.init();

    // uint32_t failed_panel = 0;
    // if(panel_ctl.init_all(failed_panel) != LedPanelStatus::OK)
    // {
    //     msg_t error_msg;
    //     error_msg.type = MSG_TYPE_HARDWARE_ERROR;
    //     error_msg.payload.hardware_error.error_type = 0x00;
    //     error_msg.payload.hardware_error.info = failed_panel;
    //     handle_app_error(AppStatus::NOT_OK, error_msg);
    //     return;
    // }

    // This is a good spot for any random tests during dev
    // send_ping();


    // State is complete 
    complete_state();
}

void App::run_FANCY_STARTUP(void)
{

    if(current_substate == NULL_STATE)
    {
        current_substate = static_cast<uint32_t>(FancyStartupSubstate::START_FIRST_FLASH);
    }

    FancyStartupSubstate substate = static_cast<FancyStartupSubstate>(current_substate);
    using enum FancyStartupSubstate;
    switch(substate)
    {
        case START_FIRST_FLASH:
            substate_timer.set(FIRST_FLASH_TIME_MS);
            // usr_leds.blink_led(UsrLedIndex::LED_0, 50);
            // usr_leds.turn_on(UsrLedIndex::LED_0);
            substate = RUN_FIRST_FLASH;
            break;
        case RUN_FIRST_FLASH:
            if(substate_timer.expired())
            {
                // usr_leds.stop_blinking_led(UsrLedIndex::LED_0);
                substate = SECOND_FLASH;
            }
            break;
        case SECOND_FLASH:
            // usr_leds.turn_off(UsrLedIndex::LED_0);
            substate = COMPLETE;
            break;
        case COMPLETE:
            complete_state();
        default:
            
            break;
    }
    // Cleanup, set current_substate
    current_substate = static_cast<uint32_t>(substate);

}




//Communication stuff

void App::send_ping()
{
    msg_t ping_msg;
    ping_msg.type = MSG_TYPE__PING;
    ping_msg.payload.ping.reserved_1 = 0xBEEFu;
    ping_msg.payload.ping.reserved_2 = 0xABBAu;

    send_msg(ping_msg);
    return;
}


void App::handle_heartbeat()
{
    if(heartbeat_timer.expired())
    {
        msg_t heartbeat_msg = {};
        heartbeat_msg.type = MSG_TYPE__HEARTBEAT; 
        // heartbeat_msg.payload.heartbeat.timestamp_ms = clock.get_ms();
        // usr_leds[3].toggle();
        // usr_leds.flash(UsrLedIndex::LED_3, 2, 160);
        send_msg(heartbeat_msg);
    }

}

// void App::handle_panel_cmd(LedPanelCMD_t& cmd)
// {
//     if(cmd.led_panel_index < 0 || cmd.led_panel_index >= NUM_LED_PANELS)
//     {
//         //TODO error handling
//         return; //failure
//     }

//     panel_ctl[cmd.led_panel_index].set_duty(cmd.panel_percent);
//     return;
// }

void App::send_msg(const msg_t &msg)
{
    encoded_msg_t encoded_msg;
    encode(&msg, &encoded_msg);
    can_msg_t can_msg;
    convert_to_can(&encoded_msg, &can_msg);
    if(can.send(can_msg) != CanStatus::OK)
    {
        handle_app_error(AppStatus::CAN_SEND_ERROR);
    }
    return;

}


void App::handle_app_error(AppStatus error_or_status, msg_t error_msg)
{
    // usr_leds.turn_all_off();
    // usr_leds.blink_led(UsrLedIndex::LED_0, 500);
    // usr_leds[0].toggle();
    // usr_leds.blink_led(UsrLedIndex::LED_1, 500);

    if(error_msg.type != MSG_TYPE__UNKNOWN)
    {
        send_msg(error_msg);
    }
    msg_t generic_err_msg;
    generic_err_msg.type = MSG_TYPE__UNKNOWN;
    // usr_leds.turn_all_off();
    UNUSED(error_or_status);
    for(;;)
    {
        // usr_leds.update();
        clock.blocking_delay(500);
        send_msg(generic_err_msg);

        // Get stuck here
    }
}