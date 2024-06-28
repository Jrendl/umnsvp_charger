
#pragma once

#include "hal.h"
#include "pwm_driver.h"

static GPIO_TypeDef* const PROX_PORT = GPIOA;
constexpr uint16_t PROX_PIN = GPIO_PIN_6;

static GPIO_TypeDef* const CONTROL_PORT = GPIOA;
constexpr uint16_t CONTROL_PIN = GPIO_PIN_4;

namespace umnsvp {
namespace charger {
/**
 * @brief Abstraction for communication between J1772 and charger board.
 *
 */
class J1772 {
   private:
    pwm_driver pwm;

   public:
    J1772() {
    }
    void measure_control_pilot_duty();
    float get_j1772_current_limit();
    TIM_HandleTypeDef* get_pwm_timer_handle();
    void init();
    bool check_prox_connected();
    void output_ac();
    void isolate_interface();
};

}  // namespace charger
}  // namespace umnsvp