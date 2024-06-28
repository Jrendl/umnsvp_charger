#pragma once
#include "battery_charging_limits.h"

namespace umnsvp {
namespace charger {

// Calculated by max battery cell voltage * numbers of series cells
static constexpr float CHARGER_VOLTAGE_MAX =
    MAX_CELL_VOLTAGE * NUM_SERIES_CELLS;  // volts
// Temperature is given by the thunderstruck documentation
static constexpr float CHARGER_TEMP_MAX = 54.0;  // Celsius
// Charger efficiency is given by the thunderstruck documentation
static constexpr float CHARGER_EFFICIENCY = 0.95;  // Percent
// Number of chargers being used
static constexpr float NUMBER_CHARGERS = 2;
// Allows a distinction between level 1 and level 2 charging
static constexpr float AC_VOLTAGE_INPUT_1 = 110;  // AC Volt
static constexpr float AC_VOLTAGE_INPUT_2 = 240;  // AC volts
// The ability to charge over this current indicates level 2 charging
static constexpr float AC_VOLTAGE_CHANGE_POINT = 17.3;  // Amps
}  // namespace charger
}  // namespace umnsvp