#include "timing.h"

#include "main.h"
#include "pwm_driver.h"

// this is where we'll put timer inits and configs
// see lights/timing.cc

TIM_HandleTypeDef htim7;
TIM_HandleTypeDef htim6;
namespace umnsvp {
namespace charger {
/**
 * @brief Driver level initalization of timer peripheral.
 *
 * @param USER_TIM_PeriodElapsedCallback
 */
void start_car_can_send_timer(
    pTIM_CallbackTypeDef USER_TIM_PeriodElapsedCallback) {
    // Timer 7 uses APB1 clock source for internal clock
    // Set priorities to 4 for higher priority compared to the interrupts
    HAL_NVIC_SetPriority(TIM7_IRQn, 4, 4);
    HAL_NVIC_EnableIRQ(TIM7_IRQn);
    __HAL_RCC_TIM7_CLK_ENABLE();

    TIM_MasterConfigTypeDef sMasterConfig = {0};
    htim7.Instance = TIM7;
    // This clock setup is for an interrupt every 1s with an APB clock of 80
    // mHZ
    htim7.Init.Prescaler = 40000;
    htim7.Init.Period = 2000;
    htim7.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim7.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
    // Init timer
    if (HAL_TIM_Base_Init(&htim7) != HAL_OK) {
        while (1)
            ;
    }
    // Register the timer callback function
    if (HAL_TIM_RegisterCallback(&htim7, HAL_TIM_PERIOD_ELAPSED_CB_ID,
                                 USER_TIM_PeriodElapsedCallback) != HAL_OK) {
        while (1)
            ;
    }
    // Clear the IT flag before starting the timer interrupt
    __HAL_TIM_CLEAR_IT(&htim7, TIM_IT_UPDATE);
    sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
    sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
    if (HAL_TIMEx_MasterConfigSynchronization(&htim7, &sMasterConfig) !=
        HAL_OK) {
        while (1)
            ;
    }
    // Start the timer interrupt
    HAL_TIM_Base_Start_IT(&htim7);
}

/**
 * @brief Starts the timer for the can packet sending clock. This clock is
 * responsible for the sending of can packets at specific interval
 *
 * @param USER_TIM_PeriodElapsedCallback The callback function for the interrupt

 */
void start_charger_can_send_timer(
    pTIM_CallbackTypeDef USER_TIM_PeriodElapsedCallback) {
    // Timer 7 uses APB1 clock source for internal clock
    // Set priorities to 4 for higher priority compared to the other interrupts
    HAL_NVIC_SetPriority(TIM6_DAC_IRQn, 8, 8);
    HAL_NVIC_EnableIRQ(TIM6_DAC_IRQn);
    __HAL_RCC_TIM6_CLK_ENABLE();

    TIM_MasterConfigTypeDef sMasterConfig = {0};
    htim6.Instance = TIM6;
    // This clock setup is for an interrupt every 100ms with an APB clock of 80
    // mHZ
    htim6.Init.Prescaler = 40000;
    htim6.Init.Period = 200;
    htim6.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim6.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
    // Init timer
    if (HAL_TIM_Base_Init(&htim6) != HAL_OK) {
        while (1)
            ;
    }
    // Register the timer callback function
    if (HAL_TIM_RegisterCallback(&htim6, HAL_TIM_PERIOD_ELAPSED_CB_ID,
                                 USER_TIM_PeriodElapsedCallback) != HAL_OK) {
        while (1)
            ;
    }
    // Clear the IT flag before starting the timer interrupt
    __HAL_TIM_CLEAR_IT(&htim6, TIM_IT_UPDATE);
    sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
    sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
    if (HAL_TIMEx_MasterConfigSynchronization(&htim6, &sMasterConfig) !=
        HAL_OK) {
        while (1)
            ;
    }
    // Start the timer interrupt
    HAL_TIM_Base_Start_IT(&htim6);
}

}  // namespace charger
}  // namespace umnsvp
