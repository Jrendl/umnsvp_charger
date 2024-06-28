#pragma once

#include <limits>

#include "application_base.h"
#include "battery_charging_limits.h"
#include "bms.h"
#include "bxcan.h"
#include "j1772.h"
#include "skylab2_boards.h"
#include "status_lights.h"
#include "thunderstruck.h"
#include "thunderstruck_constants.h"

namespace umnsvp {
namespace charger {
/**
 * @brief States for charger state machine.
 * IDLE: Proximity is disconnected.
 * CONNECTED: Proximity is connected, but we haven't sent a charge request yet.
 * THUNDERSTRUCK_POWER_ON: Waiting for response from the thunderstrucks.
 * CHARGING: Actively charging the battery.
 * FAULT_LATCHING: Charging has either hit a fault from BMS or the
 * thunderstrucks that must be cleared manually FAULT_RESETTABLE: Charging has
 * hit a fault that may be cleared by unplugging the charging port
 * CHARGING_DONE: We have hit charging limit.
 *
 */
enum class charge_state : uint16_t
{
    IDLE,
    CONNECTED,
    THUNDERSTRUCK_POWER_ON,
    CHARGING,
    FAULT_LATCHING,
    FAULT_RESETTABLE,
    CHARGING_DONE
};
// hard maximum on ac input current
static constexpr float MAX_AC_CURRENT = 30.0f;
// maximum time to wait for current to drop low for hv isolation
static constexpr uint32_t MAX_ISOLATE_WAIT = 500;  // ms

/**
 * @brief Application class for charger. Encapsulates all neccessary functions.
 *
 */
class Application : public ApplicationBase {
   private:
    static constexpr float VOLTAGE_THRESHOLD =
        2.0f;  // used to compare voltage target voltage -
               // bms.get_pack_voltage()
    static constexpr float CURRENT_MIN_VAL =
        30.0f;  // temporary place holder value; TODO: will be replaced with the
                // current limit packet from bms once that is implemented
    charge_state charge_status = charge_state::IDLE;
    bms_fault_type bms_fault_reason = bms_fault_type::NONE;
    charger_fault_type charger_fault_reason = charger_fault_type::NONE;
    float user_defined_current = std::numeric_limits<float>::max();
    float max_kWh = 20.15;

    can::bxcan_driver can_device;
    skylab2::charger_can skylab2;
    Bms bms;
    Thunderstruck thunderstruck;
    Status_lights status_lights;
    J1772 openEVSE;

    void init();
    void enable_charge();
    void set_current_limit();
    void HV_isolate();
    float find_current_limit(void);
    // void check_user_defined_values(); will not be adding to development
    // because untested

   public:
    Application();
    void main();
    void check_faults();
    void update_state_connected();
    void update_state_disconnected();
    void state_action();
    void broadcast_charger_can();
    void broadcast_car_can();
    void pwm_measure();

    void can1_rx_callback(void);
    void can1_tx_callback(void);

    CAN_HandleTypeDef* get_can1_handle();

    void can2_rx_callback(void);
    void can2_tx_callback(void);

    CAN_HandleTypeDef* get_can2_handle();

    TIM_HandleTypeDef* get_pwm_handle();
};
}  // namespace charger
}  // namespace umnsvp
