#include "status_lights.h"

#include "hal.h"

namespace umnsvp {
namespace charger {

/**
 * @brief This initalizes the GPIOs neccessary for all LEDs on the charger
 * board.
 *
 */
void Status_lights::init() {
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();
    // Pack the pin configuration into a struct.
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = PIN_COMM_LIGHT_1;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;

    // Configure the GPIO port with the packed pin configuration.
    HAL_GPIO_Init(PORT_COMM_LIGHT_1, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = PIN_COMM_LIGHT_2;
    HAL_GPIO_Init(PORT_COMM_LIGHT_2, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = PIN_COMM_LIGHT_3;
    HAL_GPIO_Init(PORT_COMM_LIGHT_3, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = PIN_COMM_LIGHT_4;
    HAL_GPIO_Init(PORT_COMM_LIGHT_4, &GPIO_InitStruct);

    // init light
    GPIO_InitStruct.Pin = CHARGER_CONTACTOR_CLOSE_INDICATOR_LED;
    HAL_GPIO_Init(PORT_CHARGER_CONTACTOR_LED, &GPIO_InitStruct);
    HAL_GPIO_WritePin(PORT_CHARGER_CONTACTOR_LED,
                      CHARGER_CONTACTOR_CLOSE_INDICATOR_LED, GPIO_PIN_SET);

    // PROX LED
    GPIO_InitStruct.Pin = PROX_LIGHT_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
    GPIO_InitStruct.Speed = GPIO_SPEED_MEDIUM;
    HAL_GPIO_Init(PROX_LIGHT_PORT, &GPIO_InitStruct);
    HAL_GPIO_WritePin(PROX_LIGHT_PORT, PROX_LIGHT_PIN, GPIO_PIN_SET);

    return;
}
/** @brief sets the signal light to indicate a fault state*/
void Status_lights::indicate_fault() {
    HAL_GPIO_WritePin(PORT_COMM_LIGHT_3, PIN_COMM_LIGHT_3, GPIO_PIN_RESET);
}
/** @brief toggles charger CAN signal light to indicate communcation*/
void Status_lights::toggle_charger_can_light() {
    HAL_GPIO_TogglePin(PORT_COMM_LIGHT_2, PIN_COMM_LIGHT_2);
}
/** @brief toggles car CAN signal light to indicate communication*/
void Status_lights::toggle_car_can_light() {
    HAL_GPIO_TogglePin(PORT_COMM_LIGHT_1, PIN_COMM_LIGHT_1);
}
/** @brief sets light to indicate j1772 proxy is connected*/
void Status_lights::indicate_proxy_connected() {
    HAL_GPIO_WritePin(PROX_LIGHT_PORT, PROX_LIGHT_PIN, GPIO_PIN_RESET);
}
/** @brief sets light to indicate j1772 proxy is disconnected*/
void Status_lights::indicate_proxy_disconnected() {
    HAL_GPIO_WritePin(PROX_LIGHT_PORT, PROX_LIGHT_PIN, GPIO_PIN_SET);
}
/** @brief sets light to indicate j1772 ac contactor is connected*/
void Status_lights::indicate_ac_connected() {
    HAL_GPIO_WritePin(PORT_CHARGER_CONTACTOR_LED,
                      CHARGER_CONTACTOR_CLOSE_INDICATOR_LED, GPIO_PIN_RESET);
}
/** @brief sets light to indicate j1772 ac contactor is isolated*/
void Status_lights::indicate_ac_isolated() {
    HAL_GPIO_WritePin(PORT_CHARGER_CONTACTOR_LED,
                      CHARGER_CONTACTOR_CLOSE_INDICATOR_LED, GPIO_PIN_SET);
}

}  // namespace charger
}  // namespace umnsvp
