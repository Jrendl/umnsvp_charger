/**
 * @file comm_lights.h
 * @author Alexander Vanasse (vanas073@umn.edu)
 * @brief Header for charger Comm_lights class
 * @version 1
 * @date 2022-12-5
 *
 * @copyright Copyright (c) 2022
 *
 */
#pragma once

#include "hal.h"

static GPIO_TypeDef* const PORT_COMM_LIGHT_1 = GPIOB;
constexpr uint16_t PIN_COMM_LIGHT_1 = GPIO_PIN_14;

static GPIO_TypeDef* const PORT_COMM_LIGHT_2 = GPIOB;
constexpr uint16_t PIN_COMM_LIGHT_2 = GPIO_PIN_15;

static GPIO_TypeDef* const PORT_COMM_LIGHT_3 = GPIOC;
constexpr uint16_t PIN_COMM_LIGHT_3 = GPIO_PIN_6;

static GPIO_TypeDef* const PORT_COMM_LIGHT_4 = GPIOC;
constexpr uint16_t PIN_COMM_LIGHT_4 = GPIO_PIN_7;

static GPIO_TypeDef* const PROX_LIGHT_PORT = GPIOA;
constexpr uint16_t PROX_LIGHT_PIN = GPIO_PIN_1;

static GPIO_TypeDef* const PORT_CHARGER_CONTACTOR_LED = GPIOA;
constexpr uint16_t CHARGER_CONTACTOR_CLOSE_INDICATOR_LED = GPIO_PIN_2;
namespace umnsvp {
namespace charger {

/** @brief wrapper for charger board communication lights*/
class Status_lights {
   public:
    void init();
    void indicate_fault();
    void toggle_charger_can_light();
    void toggle_car_can_light();

    void indicate_proxy_connected();
    void indicate_proxy_disconnected();

    void indicate_ac_connected();
    void indicate_ac_isolated();
};

}  // namespace charger
}  // namespace umnsvp