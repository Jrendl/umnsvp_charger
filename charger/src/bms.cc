#include "bms.h"

namespace umnsvp {
namespace charger {

Bms::Bms(skylab2::charger_can &skylab2) : skylab2(skylab2) {
}

void Bms::set_charging_request_true() {
    charging_requested = true;
}

void Bms::set_charging_request_false() {
    charging_requested = false;
}

/**
 * @brief Send request to charge/not charge to BMS.
 *
 */
void Bms::can_send_charging_request_status(void) {
    skylab2::can_packet_charger_bms_request msg;
    msg.request_flags.charging_requested = charging_requested;
    skylab2.send_charger_bms_request(msg);
}

float Bms::get_max_cell_voltage() const {
    return max_cell_voltage;
}

float Bms::get_max_cell_temp() const {
    return max_cell_temp;
}

float Bms::get_pack_voltage() const {
    return pack_voltage;
}

bool Bms::check_battery_killed() {
    return killed;
}

bool Bms::check_ready_to_charge() {
    return charging_ready;
}

float Bms::get_battery_capacity() const {
    return current_battery_pack_capacity;
}

float Bms::get_battery_current() const {
    return battery_current;
}

/**
 * @brief Check for any faults from BMS.
 *
 * @return bms_fault_type Fault received from BMS.
 */
bms_fault_type Bms::check_current_fault() {
    // fault handling
    if (check_battery_killed()) {  // killed
        return bms_fault_type::HV_KILL;
    }

    if (get_pack_voltage() <= BATTERY_VOLTAGE_MIN) {  // undervolt
        return bms_fault_type::BATTERY_UNDERVOLT;
    }

    if (get_pack_voltage() >= BATTERY_VOLTAGE_MAX) {  // overvolt
        return bms_fault_type::BATTERY_OVERVOLT;
    }

    if (get_max_cell_temp() >=
        CHARGING_TEMP_LIMIT_FOR_BATTERY) {  // cell overtemp
        return bms_fault_type::CELL_OVERTEMP;
    }

    if (!check_comms_alive()) {
        return bms_fault_type::BMS_CAN_TIMEOUT;
    }

    return bms_fault_type::NONE;
}

/**
 * @brief Probe all CAN values coming from BMS.
 *
 */
void Bms::update_can_values() {
    // mod min max
    if (skylab2.bms_module_min_max_buffer.pop()) {
        skylab2::can_packet_bms_module_min_max msg =
            skylab2.bms_module_min_max_buffer.output();
        max_cell_voltage = msg.module_max_voltage * CELL_VOLTAGE_CONVERSION;
        max_cell_temp = msg.module_max_temp * CELL_TEMP_CONVERSION;
        received_min_max = HAL_GetTick();
    }

    // ready to charge
    if (skylab2.bms_charger_response_buffer.pop()) {
        skylab2::can_packet_bms_charger_response msg =
            skylab2.bms_charger_response_buffer.output();
        charging_ready = msg.response_flags.charging_ready == 1;
        received_charging_ready = HAL_GetTick();
    }

    // battery killed
    if (skylab2.battery_status_buffer.pop()) {
        skylab2::can_packet_battery_status msg =
            skylab2.battery_status_buffer.output();
        killed = msg.battery_state.killed;
        received_killed = HAL_GetTick();
    }

    // pack voltage
    if (skylab2.bms_measurement_buffer.pop()) {
        skylab2::can_packet_bms_measurement msg =
            skylab2.bms_measurement_buffer.output();
        pack_voltage = msg.battery_voltage * PACK_VOLTAGE_CONVERSION;
        battery_current = msg.current;
        received_pack_voltage = HAL_GetTick();
    }

    // battery capacity
    if (skylab2.bms_capacity_buffer.pop()) {
        skylab2::can_packet_bms_capacity msg =
            skylab2.bms_capacity_buffer.output();
        current_battery_pack_capacity = msg.Wh / 1000;
    }
}

/**
 * @brief Confirm that communication with BMS is still active.
 *
 * @return true If we've recently received communication.
 * @return false otherwise.
 */
bool Bms::check_comms_alive() {
    if (received_min_max.has_value() && received_charging_ready.has_value() &&
        received_killed.has_value() && received_pack_voltage.has_value()) {
        uint32_t tick = HAL_GetTick();
        return (((tick - received_min_max.value()) < TIMEOUT) &&
                ((tick - received_charging_ready.value()) < TIMEOUT) &&
                ((tick - received_killed.value()) < TIMEOUT) &&
                ((tick - received_pack_voltage.value()) < TIMEOUT));
    }
    return false;
}

}  // namespace charger
}  // namespace umnsvp