#pragma once

#include "hal.h"

namespace umnsvp {
namespace charger {
void start_car_can_send_timer(
    pTIM_CallbackTypeDef USER_TIM_PeriodElapsedCallback);
void start_charger_can_send_timer(
    pTIM_CallbackTypeDef USER_TIM_PeriodElapsedCallback);
}  // namespace charger
}  // namespace umnsvp
