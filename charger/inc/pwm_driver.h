
#pragma once

#include "hal.h"
#include "stm32f4xx_hal_conf.h"
#include "stm32f4xx_hal_gpio.h"

namespace umnsvp {
namespace charger {
/**
 * @brief Class handles PWM and duty cycle.
 *
 */

class pwm_driver {
   private:
    void duty_cycle_measure_timer_init();
    TIM_HandleTypeDef htim_handle = {0};
    float duty_cycle;

   public:
    void init();
    void measure_duty();
    uint16_t get_capture_period();
    TIM_HandleTypeDef* get_handler();
    float get_frequency();
    float get_duty() const;
    static constexpr uint64_t timer_ref_clock =
        800000;  // reference clock for timer 1 (in MHz)
};
}  // namespace charger
}  // namespace umnsvp
