#include "usr_led.hpp"
//TODO add more descripive enums eg. UsrLedState or smt.

using enum UsrLedStatus;

UsrLedStatus UsrLedControl::init()
{
    for(auto& led : this->leds)
    {
        if(led.init() != LED_OK)
        {
            return LED_ERROR;
        }
    }
    for(auto& timer : this->blinking_timers)
    {
        if(timer.init() != true)
        {
            return LED_ERROR;
        }
    }
    this->ready = true;
    return LED_OK;
}

UsrLedStatus UsrLedControl::turn_all_off()
{
    for(auto& led : this->leds)
    {
        if(led.turn_off() != LED_OK)
        {
            return LED_ERROR;
        }
    }
    return LED_OK;
}

UsrLedStatus UsrLedControl::turn_all_on()
{
    for(auto& led : this->leds)
    {
        if(led.turn_on() != LED_OK)
        {
            return LED_ERROR;
        }
    }
    return LED_OK;
}

UsrLedStatus UsrLedControl::turn_on(UsrLedIndex index)
{
    if(leds[static_cast<size_t>(index)].turn_on() != LED_OK)
    {
        return LED_ERROR;
    }
    return LED_OK;
}

UsrLedStatus UsrLedControl::turn_off(UsrLedIndex index)
{
    if(leds[static_cast<size_t>(index)].turn_off() != LED_OK)
    {
        return LED_ERROR;
    }
    return LED_OK;
}

UsrLedStatus UsrLedControl::blink_led(UsrLedIndex index, uint32_t period_ms)
{
    if(this->ready == false)
    {
        return LED_ERROR;
    }
    size_t i = static_cast<size_t>(index);
    uint32_t half_period_ms = period_ms/2; // Divide by 2
    blinking_timers[i].set(half_period_ms);
    blinking[i] = true;
    turn_on(index); // start on
    return LED_OK;

}

UsrLedStatus UsrLedControl::stop_blinking_led(UsrLedIndex index)
{
    if(this->ready == false)
    {
        return LED_ERROR;
    }
    size_t i = static_cast<size_t>(index);
    blinking_timers[i].disable();
    blinking[i] = false;
    turn_off(index);

    return LED_OK;
}

void UsrLedControl::update()
{
    for(int i = 0; i < NUM_USR_LEDS; i++)
    {
        // Check blinking
        if(blinking[i])
        {
            if(blinking_timers[i].expired())
            {
                leds[i].toggle();
                if(flashes_remaining[i] > 1 && leds[i].get_state() == UsrLed::ON)
                {
                    // Only decrement when led is toggled ON (so, once every 2 timer expirys)
                    flashes_remaining[i]--;
                } else if(flashes_remaining[i] == 1)
                {
                    flashes_remaining[i] = 0;
                    stop_blinking_led(static_cast<UsrLedIndex>(i));
                }
            }
        }
    }
}

UsrLedStatus UsrLedControl::flash(UsrLedIndex index, uint32_t times, uint32_t period_ms)
{
    size_t i = static_cast<size_t>(index);
    flashes_remaining[i] = times;
   return blink_led(index, period_ms);
}

UsrLedStatus UsrLed::init()
{
    if(this->led_output.init())
    {
        this->ready = true;
        this->turn_off(); // Start with LED off.
        return LED_OK;
    }
    return LED_ERROR;
}

bool UsrLed::get_state()
{
    return this->state;
}

UsrLedStatus UsrLed::turn_off()
{
    return this->set(OFF);
}

UsrLedStatus UsrLed::turn_on()
{
    return this->set(ON);
}

UsrLedStatus UsrLed::set(bool state)
{
    if(this->ready != true)
    {
        return LED_ERROR;
    }
    bool state_to_write;
    if(active_low)
    {
        state_to_write = !state;
    } else 
    {
        state_to_write = state;
    }

    if(this->led_output.write(state_to_write) != true)
    {
        return LED_ERROR;
    }
    // If success, update internal state
    this->state = state;
    return LED_OK;
}

UsrLedStatus UsrLed::toggle()
{
    return this->set(!state);
}