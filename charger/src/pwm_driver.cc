
#include "pwm_driver.h"

#include "hal.h"
#include "main.h"

namespace umnsvp {
namespace charger {

TIM_HandleTypeDef* pwm_driver::get_handler() {
    return &htim_handle;
}

/**
 * @brief This function initalizes the duty cycle timer
 *
 */
void pwm_driver::init() {
    duty_cycle_measure_timer_init();
    HAL_TIM_IC_Start_IT(&htim_handle, TIM_CHANNEL_1);  // main channel
    HAL_TIM_IC_Start(&htim_handle, TIM_CHANNEL_2);
}

/**
 * @brief This function calculates the frequency of the duty cycle
 * using the captured period and timer 1's reference clock.
 *
 * @return float The frequency (Hz).
 */
float pwm_driver::get_frequency() {
    uint16_t captured_period = get_capture_period();
    return (static_cast<float>(timer_ref_clock) / captured_period);
}

/**
 * @brief This function returns the captured period of
 * channel 1 (low and high) in periods.
 *
 * @return uint16_t The captured period.
 */
uint16_t pwm_driver::get_capture_period() {
    uint16_t captured_period = 0;
    captured_period = HAL_TIM_ReadCapturedValue(&htim_handle, TIM_CHANNEL_1);
    return captured_period;
}

/**
 * @brief This function calculates the duty cycle by
 * subtracting the percentage of the low of the period
 * by one (AKA 100 percent of the period).
 *
 */
void pwm_driver::measure_duty() {
    uint16_t captured_period = get_capture_period();
    float calculated_duty_cycle = 0.0;
    float read_capture_value =
        (HAL_TIM_ReadCapturedValue(&htim_handle, TIM_CHANNEL_2));
    calculated_duty_cycle =
        1.00 - (read_capture_value / captured_period);  // this is a decimal and
                                                        // not a percentage
    duty_cycle = calculated_duty_cycle;
}

float pwm_driver::get_duty() const {
    return duty_cycle;
}

/**
 * @brief Initalizes the GPIO interface for control pin.
 *
 */
void pwm_driver::duty_cycle_measure_timer_init() {
    TIM_SlaveConfigTypeDef sSlaveConfig = {0};
    TIM_IC_InitTypeDef sConfigIC = {0};
    TIM_MasterConfigTypeDef sMasterConfig = {0};

    GPIO_InitTypeDef GPIO_InitStruct = {0};

    __HAL_RCC_TIM1_CLK_ENABLE();

    __HAL_RCC_GPIOA_CLK_ENABLE();

    GPIO_InitStruct.Pin = GPIO_PIN_8;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF1_TIM1;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    htim_handle.Instance = TIM1;
    htim_handle.Init.Prescaler = 100;
    htim_handle.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim_handle.Init.Period = 65535;
    htim_handle.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim_handle.Init.RepetitionCounter = 0;
    htim_handle.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
    if (HAL_TIM_IC_Init(&htim_handle) != HAL_OK) {
        while (1)
            ;
    }
    sSlaveConfig.SlaveMode = TIM_SLAVEMODE_RESET;
    sSlaveConfig.InputTrigger = TIM_TS_TI1FP1;  // could be something
    sSlaveConfig.TriggerPolarity = TIM_INPUTCHANNELPOLARITY_RISING;
    sSlaveConfig.TriggerPrescaler = TIM_ICPSC_DIV1;
    sSlaveConfig.TriggerFilter = 0;
    if (HAL_TIM_SlaveConfigSynchro(&htim_handle, &sSlaveConfig) != HAL_OK) {
        while (1)
            ;
    }
    sConfigIC.ICPolarity = TIM_INPUTCHANNELPOLARITY_RISING;
    sConfigIC.ICSelection = TIM_ICSELECTION_DIRECTTI;
    sConfigIC.ICPrescaler = TIM_ICPSC_DIV2;
    sConfigIC.ICFilter = 0;
    if (HAL_TIM_IC_ConfigChannel(&htim_handle, &sConfigIC, TIM_CHANNEL_1) !=
        HAL_OK) {
        while (1)
            ;
    }
    sConfigIC.ICPolarity = TIM_INPUTCHANNELPOLARITY_FALLING;
    sConfigIC.ICSelection = TIM_ICSELECTION_INDIRECTTI;
    if (HAL_TIM_IC_ConfigChannel(&htim_handle, &sConfigIC, TIM_CHANNEL_2) !=
        HAL_OK) {
        while (1)
            ;
    }
    sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
    sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
    if (HAL_TIMEx_MasterConfigSynchronization(&htim_handle, &sMasterConfig) !=
        HAL_OK) {
        while (1)
            ;
    }

    HAL_NVIC_SetPriority(TIM1_CC_IRQn, 7, 7);
    HAL_NVIC_EnableIRQ(TIM1_CC_IRQn);
    if (HAL_TIM_RegisterCallback(&htim_handle, HAL_TIM_IC_CAPTURE_CB_ID,
                                 &timer_handler_callback) != HAL_OK) {
        while (1)
            ;
    }
}

}  // namespace charger
}  // namespace umnsvp