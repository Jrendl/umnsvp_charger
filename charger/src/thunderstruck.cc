#include "thunderstruck.h"

namespace umnsvp {
namespace charger {

Thunderstruck::Thunderstruck() : CANDevice() {
}

/**
 * @brief Set the limit of chargers with the resolution of 100mV.
 *
 * @param limit
 */
void Thunderstruck::set_charging_voltage_limit(float limit) {
    uint16_t voltage_int_limit =
        static_cast<uint16_t>(limit * VOLTAGE_LIMIT_INT_CONVERSION);
    control_packet.CHARGE_VOLTAGE = voltage_int_limit;
}

/**
 * @brief Set the charging current limit with the resolution of 100mA
 *
 * @param limit
 */
void Thunderstruck::set_charging_current_limit(float limit) {
    uint16_t current_int_limit =
        charging_current_packet_offset -
        static_cast<uint16_t>(limit * CURRENT_LIMIT_INT_CONVERSION);
    control_packet.CHARGE_CURRENT = current_int_limit;
}

/**
 * @brief Enables the chargers
 *
 */
void Thunderstruck::enable_charging(void) {
    control_packet.Enable = CONTROL_PACKET_ENABLE;
}

void Thunderstruck::disable_charging(void) {
    control_packet.Enable = CONTROL_PACKET_DISABLE;
    // reset charger timeout indicator
    received_status.reset();
}

/**
 * @brief Call this in a timer to send the packet
 *
 */
void Thunderstruck::send_control_packet() {
    CANDevice.send_thunderstruck_control_message(control_packet);
}

/**
 * @brief Probe for new values on the status packet.
 *
 */
void Thunderstruck::receive_status_packet() {
    if (CANDevice.thunderstruck_status_message_buffer.pop()) {
        skylab2::can_packet_thunderstruck_status_message msg =
            CANDevice.thunderstruck_status_message_buffer.output();

        charging_voltage = msg.OUTPUT_VOLTAGE / 10.0;
        charging_current =
            (charging_current_packet_offset - msg.OUTPUT_CURRENT) / 10.0;
        charger_temp = msg.CHARGER_TEMP - 40;
        received_status = HAL_GetTick();
    }
}

/**
 * @brief Returns the charging current as reported by the thunderstrucks
 *
 * @return float
 */
float Thunderstruck::get_charging_current() {
    return charging_current;
}

/**
 * @brief Returns the charging voltage as reported by the thunderstrucks
 *
 * @return float
 */
float Thunderstruck::get_charging_voltage() {
    return charging_voltage;
}

/**
 * @brief Returns the charging voltage as reported by the thunderstrucks
 *
 * @return float
 */
float Thunderstruck::get_charger_temp() {
    if (charger_temp.has_value()) {
        return charger_temp.value();
    }
    return 0;
}

/**
 * @brief Checks for faults received by chargers.
 *
 * @return charger_fault_type
 */
charger_fault_type Thunderstruck::check_current_fault() {
    if (get_charging_voltage() >= CHARGER_VOLTAGE_MAX) {  // charger overvoltage
        return charger_fault_type::CHARGER_OVERVOLT;
    }

    // if coms are dead assume the charger has had time to cool down
    // to a low temp
    if (!coms_alive())
        charger_temp = std::nullopt;
    if (charger_temp.has_value()) {  // charge overtemp
        if (charger_temp.value() >= CHARGER_TEMP_MAX) {
            return charger_fault_type::CHARGER_OVERTEMP;
        }
    }

    // don't want to fault if the charger hasn't been connected yet, just if it
    // times out when it has
    if (received_status.has_value() && !coms_alive()) {  // charger can timeout
        return charger_fault_type::CHARGER_CAN_TIMEOUT;
    }

    return charger_fault_type::NONE;
}

/**
 * @brief Check if we have recently received communications from the chargers.
 *
 * @return true If we have received communication within the time value.
 * @return false otherwise
 */
bool Thunderstruck::coms_alive() {
    if (received_status.has_value()) {
        uint32_t tick = HAL_GetTick();
        return ((tick - received_status.value()) < TIMEOUT);
    }
    return false;
}

/**
 * @brief Initializes the charger class by starting the CAN device
 *
 */
void Thunderstruck::init() {
    CANDevice.start();
}

CAN_HandleTypeDef *Thunderstruck::get_can_handle() {
    return CANDevice.get_handle();
}

void Thunderstruck::receive_callback() {
    CANDevice.receive();
}

void Thunderstruck::tx_callback() {
    CANDevice.tx_handler();
}

}  // namespace charger
}  // namespace umnsvp
