// TEMPORARY bringup code for the mini rover.
// Receives MSG_TYPE__MINI_ROVER_VEL_CMD over CAN and drives 6 motors through L298s
// (PWM on EN, direction on IN1/IN2, 3 motors per side sharing one IN1/IN2 pair).

#include "main.h"
#include "tim.h"
#include "fdcan.h"
#include "can_stm32g4xx.h"
#include <comms.h>

// ---- Tunables ----------------------------------------------------------------
static constexpr float    MAX_DUTY       = 0.40f;  // L298s are overvolted: NEVER exceed 40%
static constexpr float    MAX_LIN_MPS    = 1.0f;   // linear cmd that maps to MAX_DUTY (guess, tune)
static constexpr float    MAX_ANG_DEGPS  = 90.0f;  // angular cmd that maps to MAX_DUTY (guess, tune)
static constexpr uint32_t CMD_TIMEOUT_MS = 500;    // stop motors if no command for this long
static constexpr uint32_t HEARTBEAT_PERIOD_MS = 1000; // same as PTZ

// 16 MHz timer clock (HSI, no PLL) / (15+1) = 1 MHz tick, / (999+1) = 1 kHz PWM
static constexpr uint32_t PWM_PSC = 15;
static constexpr uint32_t PWM_ARR = 999;

// ---- Hardware ------------------------------------------------------------------
struct PwmCh
{
    TIM_HandleTypeDef *htim;
    uint32_t           ch;
};

// Assumes M1-M3 are left side, M4-M6 are right side. Swap entries if wiring differs.
static const PwmCh LEFT_MOTORS[3] = {
    {&htim1, TIM_CHANNEL_1}, // M1 PA8
    {&htim1, TIM_CHANNEL_3}, // M2 PA10
    {&htim4, TIM_CHANNEL_2}, // M3 PB7
};
static const PwmCh RIGHT_MOTORS[3] = {
    {&htim1, TIM_CHANNEL_2}, // M4 PA9
    {&htim4, TIM_CHANNEL_1}, // M5 PB6
    {&htim4, TIM_CHANNEL_4}, // M6 PB9
};

// LEFT_DIR1 PA0, LEFT_DIR2 PA1, RIGHT_DIR1 PA2, RIGHT_DIR2 PA3
static constexpr uint16_t LEFT_DIR1  = GPIO_PIN_0;
static constexpr uint16_t LEFT_DIR2  = GPIO_PIN_1;
static constexpr uint16_t RIGHT_DIR1 = GPIO_PIN_2;
static constexpr uint16_t RIGHT_DIR2 = GPIO_PIN_3;

static CanSTM32G4XX can(hfdcan1);

// ---- Helpers -------------------------------------------------------------------
static float clampf(float v, float lo, float hi)
{
    return v < lo ? lo : (v > hi ? hi : v);
}

static void pwm_setup(TIM_HandleTypeDef *htim)
{
    __HAL_TIM_SET_PRESCALER(htim, PWM_PSC);
    __HAL_TIM_SET_AUTORELOAD(htim, PWM_ARR);
    HAL_TIM_GenerateEvent(htim, TIM_EVENTSOURCE_UPDATE); // load PSC/ARR now
}

// cmd in [-1, 1]. Sign = direction, magnitude scaled to MAX_DUTY.
static void set_side(const PwmCh (&motors)[3], uint16_t dir1, uint16_t dir2, float cmd)
{
    cmd = clampf(cmd, -1.0f, 1.0f);

    if (cmd > 0.0f) {
        HAL_GPIO_WritePin(GPIOA, dir1, GPIO_PIN_SET);
        HAL_GPIO_WritePin(GPIOA, dir2, GPIO_PIN_RESET);
    } else if (cmd < 0.0f) {
        HAL_GPIO_WritePin(GPIOA, dir1, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(GPIOA, dir2, GPIO_PIN_SET);
    } else {
        HAL_GPIO_WritePin(GPIOA, dir1 | dir2, GPIO_PIN_RESET); // coast
    }

    float    mag     = (cmd < 0.0f ? -cmd : cmd) * MAX_DUTY;
    uint32_t compare = (uint32_t)(mag * (float)(PWM_ARR + 1u));
    for (const PwmCh &m : motors) {
        __HAL_TIM_SET_COMPARE(m.htim, m.ch, compare);
    }
}

static void drive(float lin_mps, float ang_degps)
{
    float lin = lin_mps / MAX_LIN_MPS;
    float ang = ang_degps / MAX_ANG_DEGPS;
    // Positive angular = turn left (CCW): right side faster
    set_side(LEFT_MOTORS, LEFT_DIR1, LEFT_DIR2, lin - ang);
    set_side(RIGHT_MOTORS, RIGHT_DIR1, RIGHT_DIR2, lin + ang);
}

static void send_msg(const msg_t &msg)
{
    encoded_msg_t encoded = {};
    can_msg_t     out     = {};
    if (encode(&msg, &encoded) && convert_to_can(&encoded, &out)) {
        can.send(out); // ignore errors for now (bus off / tx full just drops the frame)
    }
}

static void send_heartbeat(void)
{
    msg_t msg = {};
    msg.type = MSG_TYPE__HEARTBEAT;
    msg.payload.heartbeat.timestamp_ms = HAL_GetTick();
    send_msg(msg);
}

// ---- Entry -------------------------------------------------------------------
extern "C" void app_main(void)
{
    pwm_setup(&htim1);
    pwm_setup(&htim4);
    for (const PwmCh &m : LEFT_MOTORS)  HAL_TIM_PWM_Start(m.htim, m.ch);
    for (const PwmCh &m : RIGHT_MOTORS) HAL_TIM_PWM_Start(m.htim, m.ch);
    drive(0.0f, 0.0f);

    comms_set_device_id(DEVICE_ID__MINI_ROVER); // encode() refuses to send without this
    can.init();

    uint32_t last_cmd_ms       = HAL_GetTick();
    uint32_t last_heartbeat_ms = HAL_GetTick();

    for (;;)
    {
        can_msg_t can_msg = {};
        if (can.read(can_msg) == CanStatus::OK)
        {
            encoded_msg_t encoded = {};
            msg_t         msg     = {};
            if (convert_from_can(&can_msg, &encoded) && decode(&encoded, &msg) &&
                msg.type == MSG_TYPE__MINI_ROVER_VEL_CMD)
            {
                drive(msg.payload.mini_rover_velcmd.lin_vel_mps,
                      msg.payload.mini_rover_velcmd.ang_vel_degps);
                last_cmd_ms = HAL_GetTick();
            }
        }

        // Failsafe: stop if commands stop arriving
        if (HAL_GetTick() - last_cmd_ms > CMD_TIMEOUT_MS) {
            drive(0.0f, 0.0f);
        }

        if (HAL_GetTick() - last_heartbeat_ms >= HEARTBEAT_PERIOD_MS) {
            last_heartbeat_ms = HAL_GetTick();
            send_heartbeat();
        }
    }
}