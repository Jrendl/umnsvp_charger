#pragma once

#include <optional>

#include "battery_charging_limits.h"
#include "hal.h"
#include "limits"
#include "pwm_driver.h"
#include "skylab2_boards.h"
#include "thunderstruck.h"
#include "thunderstruck_constants.h"

namespace umnsvp {
namespace charger {
/**
 * @brief Enum describing BMS faults
 *
 */
enum class bms_fault_type : uint8_t
{
    NONE = 0,
    HV_KILL = 0b1 << 0,
    BATTERY_UNDERVOLT = 0b1 << 1,
    BATTERY_OVERVOLT = 0b1 << 2,
    CELL_OVERTEMP = 0b1 << 3,
    BMS_CAN_TIMEOUT = 0b1 << 4
};

/**
 * @brief This class is the main driver for battery communication abstraction
 * between the charger and battery.
 *
 */

class Bms {
   private:
    // conversion factor from 10mV to V for CAN packet resolution
    static constexpr float PACK_VOLTAGE_CONVERSION = 0.01;
    // conversion factor from mV to V for CAN packet resolution
    static constexpr float CELL_VOLTAGE_CONVERSION = 0.001;
    // conversion factor from 100 degree C to degree C for CAN packet resolution
    static constexpr float CELL_TEMP_CONVERSION = 0.01;
    // max milliseconds between packets before comms are considered dead
    static constexpr uint32_t TIMEOUT = 2500;

    bool charging_ready = false;
    bool charging_requested = false;
    float max_cell_voltage = MAX_CELL_VOLTAGE;              // volts
    float max_cell_temp = CHARGING_TEMP_LIMIT_FOR_BATTERY;  // Celsius
    float pack_voltage = BATTERY_VOLTAGE_MAX;               // volts
    bool killed = false;
    float current_battery_pack_capacity = 0;
    float battery_current = 0;

    std::optional<uint32_t> received_min_max = std::nullopt;
    std::optional<uint32_t> received_charging_ready = std::nullopt;
    std::optional<uint32_t> received_killed = std::nullopt;
    std::optional<uint32_t> received_pack_voltage = std::nullopt;

    skylab2::charger_can &skylab2;

   public:
    Bms(skylab2::charger_can &skylab2);
    void can_send_charging_request_status();
    void set_charging_request_true();
    void set_charging_request_false();
    bool check_ready_to_charge();
    float get_max_cell_voltage() const;
    float get_max_cell_temp() const;
    float get_pack_voltage() const;
    bool check_battery_killed();
    bms_fault_type check_current_fault();
    void update_can_values();
    bool check_comms_alive();
    float get_battery_capacity() const;
    float get_battery_current() const;
};

}  // namespace charger
}  // namespace umnsvp