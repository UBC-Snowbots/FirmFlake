#pragma once
#include <cstdint>

// Generic hobby-servo interface.
//
// Both flavours of servo are driven identically at the electrical level: a slow PWM
// (~50Hz / 20ms period) where only the HIGH time of the pulse matters (~1-2ms). What
// changes is what that pulse *means*:
//
//   POSITIONAL : pulse width -> shaft angle.  1000us = one end stop, 2000us = the other.
//   CONTINUOUS : pulse width -> rotation speed. neutral (~1500us) = stop, either side of
//                neutral = direction + speed. The servo has no idea where it is.
//
// So: one interface, one driver, one config struct, and a type tag that decides which
// setter is legal. Calling set_angle() on a continuous servo is a programming error and
// returns WRONG_SERVO_TYPE rather than silently doing something almost-right.

enum class ServoStatus
{
    OK,
    NOT_OK,           // HAL / hardware failure
    NOT_INITIALIZED,  // init() wasn't called, or it failed
    BAD_CONFIG,       // pulse range doesn't fit the PWM period, min >= max, etc
    WRONG_SERVO_TYPE, // set_angle() on a continuous servo (or set_speed() on a positional)
    NO_SENSOR         // read_sensor() with no feedback sensor attached
};

enum class ServoType : uint8_t
{
    POSITIONAL,
    CONTINUOUS
};

struct ServoConfig
{
    ServoType type = ServoType::POSITIONAL;

    // Pulse widths in microseconds, from the servo's datasheet.
    // 1000/1500/2000 is the classic safe set. Plenty of digital servos will accept
    // 500-2500 for more travel -- start narrow, widen only once you've confirmed the
    // horn isn't grinding into the end stops (you can hear it, and it cooks the servo).
    uint16_t min_pulse_us     = 1000u; // POSITIONAL: min_angle_deg | CONTINUOUS: full reverse
    uint16_t max_pulse_us     = 2000u; // POSITIONAL: max_angle_deg | CONTINUOUS: full forward
    uint16_t neutral_pulse_us = 1500u; // CONTINUOUS: stop (trim per servo!) | POSITIONAL: boot pose

    // POSITIONAL only: what the pulse limits mean in degrees. Units are whatever you want
    // really (deg, mm on a linear actuator, ...) as long as you're consistent.
    float min_angle_deg = 0.0f;
    float max_angle_deg = 180.0f;

    // CONTINUOUS only: commands landing within +/- this of neutral get snapped to neutral.
    // Cheap CR servos creep and buzz near stop; 20-50us is typical. 0 disables.
    uint16_t deadband_us = 0u;

    bool invert        = false; // flip direction without rewiring or re-horning
    bool start_enabled = true;  // init() drives neutral. false -> stays limp until enable()
};

// Sensible starting points. Copy and tweak per servo.
static constexpr ServoConfig SERVO_CONFIG_POSITIONAL_180 = {
    ServoType::POSITIONAL, 1000u, 2000u, 1500u, 0.0f, 180.0f, 0u, false, true};

static constexpr ServoConfig SERVO_CONFIG_CONTINUOUS = {
    ServoType::CONTINUOUS, 1000u, 2000u, 1500u, 0.0f, 0.0f, 30u, false, true};

// ---- pulse mapping ---------------------------------------------------------
// Kept as free functions in the interface header: they're pure maths, identical on every
// platform, and unit-testable on a host without dragging in the HAL.

static inline float servo_clampf(float v, float lo, float hi)
{
    return (v < lo) ? lo : ((v > hi) ? hi : v);
}

// POSITIONAL: angle -> pulse. Out-of-range angles clamp to the end stops.
static inline uint16_t servo_angle_to_pulse_us(const ServoConfig &cfg, float angle_deg)
{
    const float span = cfg.max_angle_deg - cfg.min_angle_deg;
    if (span <= 0.0f)
    {
        return cfg.neutral_pulse_us; // misconfigured, fail safe rather than divide by zero
    }

    float t = servo_clampf((angle_deg - cfg.min_angle_deg) / span, 0.0f, 1.0f);
    if (cfg.invert)
    {
        t = 1.0f - t;
    }

    const float pulse = static_cast<float>(cfg.min_pulse_us) +
                        t * static_cast<float>(cfg.max_pulse_us - cfg.min_pulse_us);
    return static_cast<uint16_t>(pulse + 0.5f);
}

// CONTINUOUS: speed percent (-100 full reverse .. 0 stop .. +100 full forward) -> pulse.
// The two halves are scaled independently off neutral, because neutral is almost never
// exactly halfway once you've trimmed it.
static inline uint16_t servo_speed_to_pulse_us(const ServoConfig &cfg, float speed_percent)
{
    float s = servo_clampf(speed_percent, -100.0f, 100.0f);
    if (cfg.invert)
    {
        s = -s;
    }

    const float neutral = static_cast<float>(cfg.neutral_pulse_us);
    float pulse;
    if (s >= 0.0f)
    {
        pulse = neutral + (s / 100.0f) * static_cast<float>(cfg.max_pulse_us - cfg.neutral_pulse_us);
    }
    else
    {
        pulse = neutral + (s / 100.0f) * static_cast<float>(cfg.neutral_pulse_us - cfg.min_pulse_us);
    }

    const float delta = (pulse > neutral) ? (pulse - neutral) : (neutral - pulse);
    if (delta <= static_cast<float>(cfg.deadband_us))
    {
        pulse = neutral;
    }

    return static_cast<uint16_t>(pulse + 0.5f);
}

class ServoInterface
{
public:
    virtual ServoStatus init(void) = 0;

    virtual ServoType get_type(void) const = 0;

    // POSITIONAL only. Clamps to the configured travel, returns WRONG_SERVO_TYPE otherwise.
    virtual ServoStatus set_angle(float angle_deg) = 0;

    // CONTINUOUS only. -100 .. 0 .. +100. Returns WRONG_SERVO_TYPE otherwise.
    virtual ServoStatus set_speed(float speed_percent) = 0;

    // Both. Bypasses the angle/speed mapping -- useful for calibration and for replaying a
    // pulse straight off the wire. Still clamped to [min_pulse_us, max_pulse_us].
    virtual ServoStatus set_pulse_us(uint32_t pulse_us) = 0;

    // Last commanded pulse, and the setpoint in its natural units (deg or percent).
    // Handy for answering a "what are you currently set to" query over comms.
    virtual uint32_t get_pulse_us(void) const = 0;
    virtual float    get_setpoint(void) const = 0;

    // Stop / resume driving. A servo with no pulse goes limp (positional) or coasts to a
    // stop (continuous), so disable() is also your e-stop.
    virtual ServoStatus enable(void)  = 0;
    virtual ServoStatus disable(void) = 0;
    virtual bool        is_enabled(void) const = 0;

    // Optional feedback (potentiometer wiper on a hacked servo, current sense, endstop...)
    virtual bool        has_sensor(void) const = 0;
    virtual ServoStatus read_sensor(uint32_t &raw_value) = 0;

    virtual uint8_t get_index(void) const = 0;

    virtual ~ServoInterface() = default; // NOTE: must be virtual -- the LED panel one isn't,
                                         // which is UB if you ever delete through a base ptr.
};