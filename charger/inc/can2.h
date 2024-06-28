
#pragma once

#include "bxcan.h"
#include "circular_buffer.h"
#include "hal.h"
#include "skylab2_packets.h"
#include "triple_buffer.h"

namespace umnsvp {
namespace charger {
/**
 * @brief Abstraction of charger control board and thunderstrucks.
 *
 */
class CAN2Device {
   private:
    can::bxcan_driver can_device;
    umnsvp::circular_buffer::CircularBuffer<can::packet, 75> tx_buffer;

   public:
    void start();
    void tx_handler();
    triple_buffer::TripleBuffer<
        skylab2::can_packet_thunderstruck_control_message>
        thunderstruck_control_message_buffer;
    triple_buffer::TripleBuffer<
        skylab2::can_packet_thunderstruck_status_message>
        thunderstruck_status_message_buffer;
    CAN2Device();
    CAN_HandleTypeDef* get_handle();
    void receive();
    can::status send_thunderstruck_control_message(
        skylab2::can_packet_thunderstruck_control_message msg);
    can::status send_thunderstruck_status_message(
        skylab2::can_packet_thunderstruck_status_message msg);
};

}  // namespace charger
}  // namespace umnsvp
