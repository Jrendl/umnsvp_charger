#include "j1772.h"

namespace umnsvp {
namespace charger {

/**
 * @brief Initalize all hardware interfaces to the EVSE.
 *
 */
void J1772::init() {
    // Initialize the proximity pin and LED
    __HAL_RCC_GPIOA_CLK_ENABLE();

    // Pack the pin configuration into a struct.
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = PROX_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Speed = GPIO_SPEED_FAST;

    // Configure the GPIO port with the packed pin configuration.
    HAL_GPIO_Init(PROX_PORT, &GPIO_InitStruct);

    // Initialize the pin for the prox_enable
    GPIO_InitTypeDef gpio = {0};
    gpio.Mode = GPIO_MODE_OUTPUT_PP;
    gpio.Pin = CONTROL_PIN;
    gpio.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(CONTROL_PORT, &gpio);

    // Initialize the PWM driver
    pwm.init();
    return;
}

/**
 * @brief Returns true if prox is connected, prox is low when the plug is in th
 * car
 *
 * @return true
 * @return false
 */
bool J1772::check_prox_connected() {
    return (HAL_GPIO_ReadPin(PROX_PORT, PROX_PIN) ==
            GPIO_PinState::GPIO_PIN_SET);
}

/**
 * @brief turns on the J1772 by pulling the control pin low
 *
 */
void J1772::output_ac(void) {
    HAL_GPIO_WritePin(CONTROL_PORT, CONTROL_PIN, GPIO_PIN_SET);
}

/**
 * @brief turn off high voltage connection to the J1772
 *
 */
void J1772::isolate_interface(void) {
    HAL_GPIO_WritePin(CONTROL_PORT, CONTROL_PIN, GPIO_PIN_RESET);
}

TIM_HandleTypeDef* J1772::get_pwm_timer_handle() {
    return pwm.get_handler();
}

void J1772::measure_control_pilot_duty() {
    pwm.measure_duty();
}

/**
 * @brief Read current limit given by the J1772.
 *
 * @return float The current limit.
 *
 */
float J1772::get_j1772_current_limit() {
    // assuming a 4% tolerance
    // see Table 6A/B for current limit signaling protocol in
    // https://wiki.umnsvp.org/wiki/uberwiki_files/images/f/f1/SAE_J1772-2010.pdf
    if (!check_prox_connected()) {
        return 0;
    }
    float duty_cycle = pwm.get_duty();
    if (pwm.get_frequency() <= 1400 && pwm.get_frequency() >= 600) {
        if (duty_cycle > 0.1 && duty_cycle < 0.86) {
            return duty_cycle * 60;
        }
        if (duty_cycle >= 0.86 && duty_cycle <= 0.96) {
            return (duty_cycle * 100 - 64) * 2.5;
        }
        return 0;
    } else {
        return 0;  // outside of expected range
    }
}
}  // namespace charger
}  // namespace umnsvp