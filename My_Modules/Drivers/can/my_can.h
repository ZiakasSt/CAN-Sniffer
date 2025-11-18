/**
 * @file my_can.h
 * @brief CAN configuration, buffering, and runtime handling API.
 *
 * @details
 * Provides CAN baudrate configuration functions, CAN frame
 * software buffering, and utilities for starting, stopping, filtering, and reading
 * CAN traffic. Higher-level modules (such as the settings menu or the main sniffer
 * loop) use this API to manage CAN functionality through FDCAN1.
 */

#ifndef MY_CAN_H
#define MY_CAN_H

#include <stdbool.h>
#include "my_debug.h"

/**
 * @def WAIT_FOR_TRAFFIC
 * @brief Delay (in ms) used while probing bus activity during auto-baud.
 */
#define WAIT_FOR_TRAFFIC 1500

/**
 * @def SOFTWARE_CAN_BUFFER_SIZE
 * @brief Size of the software CAN ring buffer.
 *
 * @note Must be a power of two for modulo masking to work correctly.
 */
#define SOFTWARE_CAN_BUFFER_SIZE 256

/**
 * @var hfdcan1
 * @brief Global FDCAN1 handle.
 *
 * @details
 * This handle is generated and initialized in main.c
 * by STM32CubeMX .ioc configuration file. It configures
 * the FDCAN1 peripheral used by all functions in this module.
 */
extern FDCAN_HandleTypeDef hfdcan1;

/**
 * @struct my_CAN_BitTiming
 * @brief Bit timing configuration entry for a specific CAN baud rate.
 */
typedef struct {
	uint32_t baudrate;
	uint16_t prescaler;
	uint8_t timeSeg1;
	uint8_t timeSeg2;
} my_CAN_BitTiming;

/**
 * @struct my_CAN_Status
 * @brief Tracks current CAN configuration state.
 *
 * @details
 * Used to report whether CAN is configured and what baudrate/filter settings
 * are active.
 */
typedef struct {
	bool is_set;
	uint32_t baudrate;
	uint32_t filter_id;
	uint32_t mask_id;
} my_CAN_Status;

/**
 * @struct my_CAN_Frame
 * @brief Simple software-level CAN frame representation.
 */
typedef struct {
	uint32_t Identifier;
	uint8_t DataLength;
	uint8_t Data[8];
} my_CAN_Frame;


/**
 * @var can_timings[]
 * @brief Table of supported CAN bit timings.
 */
extern const my_CAN_BitTiming can_timings[];

/**
 * @var baudrates_number
 * @brief Number of entries in can_timings[].
 */
extern const uint8_t baudrates_nbr;

/**
 * @fn my_CAN_Status my_CAN_manual_configuration(uint32_t baudrate)
 * @brief Configure CAN manually using a requested baud rate.
 *
 * @param baudrate
 * @retval Current CAN status instance
 *
 * @detail
 * If baudrate is not one of the supported bauderates,
 * CAN will not be set.
 */
my_CAN_Status my_CAN_manual_configuration(uint32_t baudrate);

/**
 * @fn my_CAN_Status my_CAN_auto_configuration(bool to_print)
 * @brief Try all supported baud rates until bus activity is detected.
 *
 * @param to_print If true, "Trying Baud Rate:<baudrate>" messages are printed.
 * 				   If false, the process is silent.
 * @retval Current CAN status instance
 *
 * @note
 * For the function to actually detect the CAN baudrate, there should be traffic
 * on the bus. If the traffic is sparse, try increase WAIT_FOR_TRAFFIC delay.
 */
my_CAN_Status my_CAN_auto_configuration(bool to_print);

/**
 * @fn my_CAN_Status my_CAN_set_filter_mask(uint32_t filter_id, uint32_t mask_id)
 * @brief Set the filter and mask values.
 *
 * @param filter_id The desired filter to be set.
 * @param mask_id The desired mask to be set.
 * @retval Current CAN status instance
 *
 * @detail
 * The desired filter and mask are set on the CAN status instance
 * and are configured at the start of the CAN sniffer.
 *
 * @note
 * They don't affect CAN baudrate auto configuration process, as it specifically
 * uses filter and mask equal to 0x000 to capture all the bus traffic.
 */
my_CAN_Status my_CAN_set_filter_mask(uint32_t filter_id, uint32_t mask_id);

/**
 * @fn my_CAN_Status get_my_CAN_status(bool to_print)
 * @brief Get the current CAN configuration status.
 *
 * @param to_print If true, current CAN status messages are printed.
 * 				   If false, nothing is printed.
 * @retval Current CAN status instance
 */
my_CAN_Status get_my_CAN_status(bool to_print);

/**
 * @fn bool my_CAN_start(void)
 * @brief Initialize CAN, configure filters, and start the CAN peripheral.
 *
 * @param None
 * @retval true If CAN is configured(baudrate is set)
 *    	   false If CAN is not configured
 *
 * @detail
 * If true is returned, CAN will start and enable interrupts for received frames
 * on FIFO0.
 */
bool my_CAN_start(void);

/**
 * @fn void my_CAN_stop(void)
 * @brief Stop CAN activity, disable interrupts, and reset software buffers.
 *
 * @param None
 * @retval None
 */
void my_CAN_stop(void);

/**
 * @fn void send_frame_over_UART(void)
 * @brief Read frames from the software buffer and print them over UART.
 *
 * @param None
 * @retval None
 *
 * @detail
 * If either software or hardware buffer overflow is detected,
 * Debug Message is printed.
 */
void send_frame_over_UART(void);

#endif /* MY_CAN_H */
