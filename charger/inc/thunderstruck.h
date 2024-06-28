#pragma once
#include <limits>
#include <optional>

#include "battery_charging_limits.h"
#include "can2.h"
#include "hal.h"
#include "thunderstruck_constants.h"

namespace umnsvp {
namespace charger {
/**
 * @brief Enum describing charger faults
 *
 */
enum class charger_fault_type : uint8_t
{
    NONE = 0,
    CHARGER_OVERVOLT = 0b1 << 0,
    CHARGER_OVERTEMP = 0b1 << 1,
    CHARGER_CAN_TIMEOUT = 0b1 << 2
};

class Thunderstruck {
   private:
    // converstion from V to 100mV for CAN packet resolution
    static constexpr uint16_t VOLTAGE_LIMIT_INT_CONVERSION = 10;
    // converstion from A to 100mA for CAN packet resolution
    static constexpr uint16_t CURRENT_LIMIT_INT_CONVERSION = 10;

    float charging_current = 0;  // AC current
    float charging_voltage = 0;  // votlage
    // this is room temp, if you have a better idea lmk
    std::optional<float> charger_temp;  // celsius
    CAN2Device CANDevice;
    skylab2::can_packet_thunderstruck_control_message control_packet = {0};

    /** offset for charging current values in skylab.
     * the charging current and charging current limit
     *  in skylab are set as skylabValue = offset - current,
     * in 100 mA
     */
    static constexpr uint16_t charging_current_packet_offset = 3200;
    static constexpr uint32_t TIMEOUT = 2000;  // CAN specific time out

    // value in CAN packet to enable thunderstruck
    static constexpr uint8_t CONTROL_PACKET_ENABLE = 0xFC;
    // value in CAN packet to enable thunderstruck
    static constexpr uint8_t CONTROL_PACKET_DISABLE = 0xFF;
    // for more info on thunderstruck CAN packets see
    // https://wiki.umnsvp.org/uberwiki/G1:Charger#TSM2500_CAN_Packet_Encoding

    std::optional<uint32_t> received_status;

   public:
    void init();
    // Interupt functions
    void receive_callback();
    void tx_callback();
    // Send function for can packets to charger
    void send_control_packet();
    void receive_status_packet();

    Thunderstruck();

    // Getters and setters for current and voltage.
    // IMPORTANT: Beware that the receiving messages from these functions won't
    // be able to differentiate between different chargers on the same bus
    // because of the shared CAN IDs
    float get_charging_voltage();
    void set_charging_voltage_limit(float limit);

    float get_charging_current();
    void set_charging_current_limit(float limit);

    float get_charger_temp();

    charger_fault_type check_current_fault(void);

    void enable_charging(void);
    void disable_charging(void);
    bool coms_alive();

    CAN_HandleTypeDef *get_can_handle();
};

}  // namespace charger
}  // namespace umnsvp
