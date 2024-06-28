#include "can2.h"

namespace umnsvp {
namespace charger {

CAN2Device::CAN2Device() : can_device(CAN2) {
}

void CAN2Device::start() {
    can_device.init(can::baud_rate::BAUD_RATE_250, true);
    can_device.start();
    can_device.filter_all();
}

void CAN2Device::tx_handler() {
    // this function may be blocking until the queue is cleared
    if (tx_buffer.peek()) {  // peek don't pop in case sending fails again
        can::status result = can_device.send(tx_buffer.output());
        if (result == can::status::OK) {
            tx_buffer.pop();  // if we were successful it's okay to pop now
        }
    } else {
        __HAL_CAN_DISABLE_IT(get_handle(), 0x01);
    }
}

/**
 * @brief Send control message.
 *
 * @param msg The message to send.
 * @return can::bxcan_driver::status
 */
can::status CAN2Device::send_thunderstruck_control_message(
    skylab2::can_packet_thunderstruck_control_message msg) {
    uint8_t data[8];
    data[0] = msg.Enable >> 0;
    data[1] = msg.CHARGE_VOLTAGE >> 0;
    data[2] = msg.CHARGE_VOLTAGE >> 8;
    data[3] = msg.CHARGE_CURRENT >> 0;
    data[4] = msg.CHARGE_CURRENT >> 8;
    data[5] = msg.LED_BLINK_PATTERN >> 0;
    data[6] = msg.RESERVED >> 0;
    data[7] = msg.RESERVED >> 8;
    // TODO: use ID enum and length constexpr instead of hardcoding values
    umnsvp::can::packet p = can::packet(
        ((uint32_t)
             skylab2::CANPacketId::CAN_PACKET_THUNDERSTRUCK_CONTROL_MESSAGE),
        ((uint8_t)skylab2::CAN_LENGTH_THUNDERSTRUCK_CONTROL_MESSAGE), data,
        true);
    return can_device.send(p);
}

/**
 * @brief Send status message.
 *
 * @param msg The message to send.
 * @return can::bxcan_driver::status
 */
can::status CAN2Device::send_thunderstruck_status_message(
    skylab2::can_packet_thunderstruck_status_message msg) {
    uint8_t data[8];
    data[0] = msg.STATUS_FLAGS >> 0;
    data[1] = msg.CHARGE_FLAGS >> 0;
    data[2] = msg.OUTPUT_VOLTAGE >> 0;
    data[3] = msg.OUTPUT_VOLTAGE >> 8;
    data[4] = msg.OUTPUT_CURRENT >> 0;
    data[5] = msg.OUTPUT_CURRENT >> 8;
    data[6] = msg.CHARGER_TEMP >> 0;
    data[7] = msg.RESERVED >> 0;
    // TODO: use ID enum and length constexpr instead of hardcoding values
    can::packet p = can::packet(
        ((uint32_t)
             skylab2::CANPacketId::CAN_PACKET_THUNDERSTRUCK_STATUS_MESSAGE),
        ((uint8_t)skylab2::CAN_LENGTH_THUNDERSTRUCK_STATUS_MESSAGE), data,
        true);
    return can_device.send(p);
}

CAN_HandleTypeDef* CAN2Device::get_handle() {
    return can_device.get_handle();
}

/**
 * @brief BxCan level driver for receiving messages on CAN2.
 *
 */
void CAN2Device::receive() {
    can::packet recv;
    // Receive out of FIFO1
    if (can_device.receive(recv, can::fifo::FIFO1) != can::status::OK) {
        return;
    }
    const uint8_t* data = recv.get_data();
    switch (recv.get_id()) {
        case ((uint32_t)skylab2::CANPacketId::
                  CAN_PACKET_THUNDERSTRUCK_CONTROL_MESSAGE): {
            skylab2::can_packet_thunderstruck_control_message msg;
            msg.Enable = (uint8_t)((data[0] << 0));
            msg.CHARGE_VOLTAGE = (uint16_t)((data[1] << 0) | (data[2] << 8));
            msg.CHARGE_CURRENT = (uint16_t)((data[3] << 0) | (data[4] << 8));
            msg.LED_BLINK_PATTERN = (uint8_t)((data[5] << 0));
            msg.RESERVED = (uint16_t)((data[6] << 0) | (data[7] << 8));
            thunderstruck_control_message_buffer.push(msg);
        } break;
        case ((uint32_t)skylab2::CANPacketId::
                  CAN_PACKET_THUNDERSTRUCK_STATUS_MESSAGE): {
            skylab2::can_packet_thunderstruck_status_message msg;
            msg.STATUS_FLAGS = (uint8_t)((data[0] << 0));
            msg.CHARGE_FLAGS = (uint8_t)((data[1] << 0));
            msg.OUTPUT_VOLTAGE = (uint16_t)((data[2] << 0) | (data[3] << 8));
            msg.OUTPUT_CURRENT = (uint16_t)((data[4] << 0) | (data[5] << 8));
            msg.CHARGER_TEMP = (uint8_t)((data[6] << 0));
            msg.RESERVED = (uint8_t)((data[7] << 0));
            thunderstruck_status_message_buffer.push(msg);
        } break;
    }
}

}  // namespace charger
}  // namespace umnsvp
