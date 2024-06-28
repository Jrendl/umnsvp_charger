#pragma once

#include "application.h"
#include "hal.h"

void timer_handler_callback(TIM_HandleTypeDef* htim);

extern umnsvp::charger::Application app;