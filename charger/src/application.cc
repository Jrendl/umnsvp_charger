#include "application.h"

#include <algorithm>

#include "battery_charging_limits.h"
#include "main.h"
#include "status_lights.h"
#include "timing.h"

namespace umnsvp {
namespace charger {

Application::Application()
    : can_device(CAN1),
      skylab2(can_device, can::fifo::FIFO0),
      bms(skylab2),
      thunderstruck(){};

/**
 * @brief Initalilzes all charger dependencies
 *
 */
void Application::init() {
    sys_init();
    status_lights.init();
    skylab2.init();
    openEVSE.init();
    thunderstruck.init();

    start_car_can_send_timer(&timer_handler_callback);
    start_charger_can_send_timer(&timer_handler_callback);
}

/**
 * @brief Implements the state machine that controls the function of the
 * charger.
 *
 */
void Application::main() {
    init();
    while (!bms.check_comms_alive()) {
        bms.update_can_values();
    }
    while (true) {
        thunderstruck.receive_status_packet();
        bms.update_can_values();

        // the only time we don't check for faults is if we're already latched
        // faulted
        if (charge_status != charge_state::FAULT_LATCHING) {
            check_faults();
        }

        if (openEVSE.check_prox_connected()) {
            status_lights.indicate_proxy_connected();
            update_state_connected();
        } else {
            status_lights.indicate_proxy_disconnected();
            update_state_disconnected();
        }
        state_action();
    }
}

/** @brief checks for faults, sets fault reason and changes to fault state when
 * applicable
 */
void Application::check_faults() {
    bms_fault_reason = bms.check_current_fault();
    charger_fault_reason = thunderstruck.check_current_fault();

    // check charger faults
    if (charger_fault_reason != charger_fault_type::NONE) {
        charge_status = charge_state ::FAULT_RESETTABLE;
    }
    // check bms faults
    switch (bms_fault_reason) {
        case bms_fault_type::NONE:
            break;
        case bms_fault_type::HV_KILL:
        case bms_fault_type::BATTERY_UNDERVOLT:
        case bms_fault_type::BMS_CAN_TIMEOUT:
            charge_status = charge_state::FAULT_LATCHING;
            break;

        case bms_fault_type::BATTERY_OVERVOLT:
        case bms_fault_type::CELL_OVERTEMP:
            charge_status = charge_state::FAULT_RESETTABLE;
            break;
        default:
            charge_status = charge_state::FAULT_LATCHING;
            break;
    }
}
/**  @brief  updates state given the evse is connected to the car*/
void Application::update_state_connected() {
    switch (charge_status) {
        case charge_state::IDLE:
            // switch to connected state only when prox connected
            charge_status = charge_state::CONNECTED;
            break;
        case charge_state::CONNECTED:
            if (bms.get_max_cell_voltage() < CELL_CHARGE_TARGET_VOLTAGE &&
                bms.get_max_cell_temp() < CHARGING_TEMP_LIMIT_FOR_BATTERY) {
                // we think its safe to charge
                if (bms.check_ready_to_charge()) {
                    // wait for bms to respond that its ok
                    charge_status = charge_state::THUNDERSTRUCK_POWER_ON;
                }
            }
            break;
        case charge_state::THUNDERSTRUCK_POWER_ON:
            if (!bms.check_ready_to_charge()) {
                // BMS doesn't want you to charge anymore,
                // don't start
                charge_status = charge_state::CONNECTED;
            } else if (thunderstruck.coms_alive()) {
                // thunderstruck on and bms still wants charging
                charge_status = charge_state::CHARGING;
            }

            break;
        case charge_state::CHARGING:
            if (!bms.check_ready_to_charge()) {
                // BMS doesn't want you to charge anymore, stop
                charge_status = charge_state::CONNECTED;
            } else if ((bms.get_battery_current() <=
                            BATTERY_CURRENT_CHARGING_TARGET &&
                        (BATTERY_VOLTAGE_CHARGING_TARGET -
                             bms.get_pack_voltage() <=
                         VOLTAGE_THRESHOLD))) {
                charge_status = charge_state::CHARGING_DONE;
            }
            break;
        case charge_state::FAULT_LATCHING:
        case charge_state::FAULT_RESETTABLE:
            // no transition out of fault when plugged in
            break;
        case charge_state::CHARGING_DONE:
            break;
        default:
            charge_status = charge_state::FAULT_LATCHING;
            break;
    }
}
/** @brief updates current state given the evse is not connected to the car*/
void Application::update_state_disconnected() {
    switch (charge_status) {
        case charge_state::FAULT_LATCHING:
            break;
        case charge_state::FAULT_RESETTABLE:
            if ((bms_fault_reason == bms_fault_type::NONE) &&
                (charger_fault_reason == charger_fault_type::NONE)) {
                charge_status = charge_state::IDLE;
            }
            break;
        default:
            charge_status = charge_state::IDLE;
            break;
    }
}
/** @brief performs the action for the current state*/
void Application::state_action() {
    // logic
    switch (charge_status) {
        case charge_state::CONNECTED:
            if (bms.get_max_cell_voltage() < CELL_CHARGE_TARGET_VOLTAGE &&
                bms.get_battery_capacity() < max_kWh &&
                bms.get_max_cell_temp() < CHARGING_TEMP_LIMIT_FOR_BATTERY) {
                bms.set_charging_request_true();
            }
            break;
        case charge_state::THUNDERSTRUCK_POWER_ON:
            openEVSE.output_ac();
            thunderstruck.set_charging_voltage_limit(BATTERY_VOLTAGE_MAX);
            status_lights.indicate_ac_connected();
            break;
        case charge_state::CHARGING:
            enable_charge();

            break;
        case charge_state::FAULT_LATCHING:
        case charge_state::FAULT_RESETTABLE:
            // HV isolate for safety while in fault
            status_lights.indicate_fault();
            HV_isolate();
            break;
        case charge_state::IDLE:
            HV_isolate();
            break;
        case charge_state::CHARGING_DONE:
            HV_isolate();
            break;
        default:
            HV_isolate();
            break;
    }
}

/**
 * @brief Turn off all high voltage power.
 *
 */
void Application::HV_isolate() {
    thunderstruck.disable_charging();
    const uint32_t timeout = HAL_GetTick() + MAX_ISOLATE_WAIT;
    // either wait 500 ms or until the current drops below 10 mA to open
    // contactors
    while ((thunderstruck.get_charging_current() > 0.01) &&
           (HAL_GetTick() <= timeout)) {
        thunderstruck.receive_status_packet();
    }

    openEVSE.isolate_interface();
    bms.set_charging_request_false();
    status_lights.indicate_ac_isolated();
}

/**
 * @brief Set limits of the thunderstrucks and send the enable packet.
 *
 */
void Application::enable_charge() {
    thunderstruck.set_charging_current_limit(find_current_limit());
    thunderstruck.set_charging_voltage_limit(BATTERY_VOLTAGE_CHARGING_TARGET);

    thunderstruck.enable_charging();
}

/**
 * @brief uses J1772 and battery limits to calculate a safe value of DC current
 * to provide to the battery from the AC input of the J1772
 *
 * @return float a safe current limit
 */
float Application::find_current_limit() {
    // check for current input from dashboard
    // check_user_defined_values();

    // check EVSE
    const float charging_current_limit = openEVSE.get_j1772_current_limit();
    const float max_ac_current =
        std::min(charging_current_limit, MAX_AC_CURRENT);

    const float ac_voltage = (max_ac_current <= AC_VOLTAGE_CHANGE_POINT)
                                 ? AC_VOLTAGE_INPUT_1
                                 : AC_VOLTAGE_INPUT_2;

    // check user_defined_values() are commented out
    // reason: not yet functional

    // check battery voltage
    const float bat_max_cell_voltage = bms.get_max_cell_voltage();

    // if (user_defined_current < 40) {
    //     // if the user defines a reasonable current, let that override the
    //     // current measurement
    //     return user_defined_current;
    // } else {

    // min of max available dc charge and linear decrease as we get to full
    // charge

    float pack_voltage = bms.get_pack_voltage();

    return std::min((CHARGER_EFFICIENCY * ac_voltage * max_ac_current) /
                        (pack_voltage * NUMBER_CHARGERS),
                    CURRENT_MIN_VAL);
}

TIM_HandleTypeDef* Application::get_pwm_handle() {
    return openEVSE.get_pwm_timer_handle();
}

/**
 * @brief Send all outgoing messages on the charger CAN Network.
 *
 */
void Application::broadcast_charger_can() {
    status_lights.toggle_charger_can_light();
    thunderstruck.send_control_packet();
}

void Application::pwm_measure() {
    openEVSE.measure_control_pilot_duty();
}

/**
 * @brief Send all outgoing CAN messages to the car on a timer.
 *
 */
void Application::broadcast_car_can() {
    status_lights.toggle_car_can_light();
    // send contactor request
    bms.can_send_charging_request_status();

    // send state message

    skylab2::can_packet_charger_state state_msg = {0};
    switch (charger_fault_reason) {
        case charger_fault_type::CHARGER_CAN_TIMEOUT:
            state_msg.fault.CHARGER_CAN_TIMEOUT = 1;
            break;
        case charger_fault_type::CHARGER_OVERTEMP:
            state_msg.fault.CHARGER_OVERTEMP = 1;
            break;
        case charger_fault_type::CHARGER_OVERVOLT:
            state_msg.fault.CHARGER_OVERVOLT = 1;
            break;
        case charger_fault_type::NONE:
        default:
            state_msg.fault.CHARGER_CAN_TIMEOUT = 0;
            state_msg.fault.CHARGER_OVERTEMP = 0;
            state_msg.fault.CHARGER_OVERVOLT = 0;
            break;
    }

    switch (bms_fault_reason) {
        case bms_fault_type::BATTERY_UNDERVOLT:
            state_msg.fault.BATTERY_UNDERVOLT = 1;
            break;
        case bms_fault_type::BATTERY_OVERVOLT:
            state_msg.fault.BATTERY_OVERVOLT = 1;
            break;
        case bms_fault_type::BMS_CAN_TIMEOUT:
            state_msg.fault.BATTERY_CAN_TIMEOUT = 1;
            break;
        case bms_fault_type::CELL_OVERTEMP:
            state_msg.fault.BATTERY_CELL_OVERTEMP = 1;
            break;
        case bms_fault_type::HV_KILL:
            state_msg.fault.BATTERY_HV_KILL = 1;
            break;
        case bms_fault_type::NONE:
        default:
            state_msg.fault.BATTERY_UNDERVOLT = 0;
            state_msg.fault.BATTERY_CAN_TIMEOUT = 0;
            state_msg.fault.BATTERY_CELL_OVERTEMP = 0;
            state_msg.fault.BATTERY_HV_KILL = 0;
            break;
    }

    state_msg.charging_current =
        thunderstruck.get_charging_current() * NUMBER_CHARGERS;

    state_msg.state_flags.charger_plugged = openEVSE.check_prox_connected();

    // conversion of 0.001
    state_msg.charger_max_temp =
        static_cast<uint16_t>(thunderstruck.get_charger_temp() * 1000);

    skylab2.send_charger_state(state_msg);
}

// user defined current limiting (and all related code to it)
// will commented out to avoid adding untested firmware to development
// void Application::check_user_defined_values() {
//     if (skylab2.charger_current_voltage_buffer.pop()) {
//         skylab2::can_packet_charger_current_voltage msg =
//             skylab2.charger_current_voltage_buffer.output();
//         user_defined_current = msg.max_current;
//         max_kWh = msg.max_capacity;
//     }
// }

// skylab necessary functions

void Application::can1_rx_callback(void) {
    skylab2.main_bus_rx_handler();
}

void Application::can1_tx_callback(void) {
    skylab2.main_bus_tx_handler();
}

CAN_HandleTypeDef* Application::get_can1_handle() {
    return can_device.get_handle();
}

void Application::can2_rx_callback(void) {
    thunderstruck.receive_callback();
}

void Application::can2_tx_callback(void) {
    thunderstruck.tx_callback();
}
CAN_HandleTypeDef* Application::get_can2_handle() {
    return thunderstruck.get_can_handle();
}

}  // namespace charger
}  // namespace umnsvp
