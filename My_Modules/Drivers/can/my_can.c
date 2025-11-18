/**
 * @file my_can.c
 * @brief CAN configuration, buffering, and runtime handling implementation.
 *
 * @details
 * Provides:
 *  - Automatic and manual CAN baudrate configuration
 *  - Software ring buffer for received frames
 *  - Filter/mask configuration
 *  - FDCAN1 start/stop control
 *  - UART forwarding of captured frames
 *
 * This module is designed to pair with an interrupt-driven FDCAN RX FIFO0
 * callback. Frames are moved into a software ring buffer to avoid losing
 * data when hardware FIFO fills up.
 */

#include "my_can.h"

/* Forward declarations for internal helpers */
static bool check_Fifo(void);
static bool read_frame_from_software_CAN_buffer(my_CAN_Frame* frame);


/**
 * @var can_timings[]
 * @brief Table of supported CAN bit-timing configurations.
 *
 * @details
 * Each entry is a tested combination of prescaler and timing segments for a
 * specific baud rate. Used for manual and auto-baud setup.
 *
 * @warning
 * These timings are calculated assuming a 40 MHz FDCAN peripheral clock and are suitable for
 * nodes that are physically close (<1–2 m), as the PROP_SEG is omitted in the calculation
 * formula. The sample point is set to 87.5% for almost each combination.
 *
 * If you plan to:
 *  - use a bus with different physical characteristics,
 *  - target a different sample point,
 *  - change the peripheral clock frequency, or
 *  - reduce TQ cycles per bit to decrease internal hardware load,
 *
 * you must manually recalculate and update these values.
 */

const my_CAN_BitTiming can_timings[] = {
		{5000, 200, 34, 5},
		{10000, 100, 34, 5},
		{20000, 50, 34, 5},
		{50000, 20, 34, 5},
		{100000, 10, 34, 5},
		{125000, 8, 34, 5},
		{200000, 5, 34, 5},
		{250000, 4, 34, 5},
		{400000, 4, 19, 5}, // Sample point 80%
		{500000, 2, 34, 5},
		{800000, 2, 19, 5}, // Sample point 80%
		{1000000, 1, 34, 5},
};

const uint8_t baudrates_nbr = sizeof(can_timings) / sizeof(can_timings[0]);

/**
 * @var software_CAN_buffer_overflow
 * @brief Flag indicating a software CAN buffer overflow.
 */
static volatile bool software_CAN_buffer_overflow = false;

/**
 * @var hardware_CAN_buffer_overflow
 * @brief Flag indicating a hardware CAN buffer overflow.
 */
static volatile bool hardware_CAN_buffer_overflow = false;

/**
 * @var head
 * @brief Software CAN ring buffer head index.
 */
static volatile uint16_t head = 0;

/**
 * @var tail
 * @brief Software CAN ring buffer tail index.
 */
static volatile uint16_t tail = 0;

/**
 * @var CAN_ring_buffer[SOFTWARE_CAN_BUFFER_SIZE]
 * @brief Software CAN ring buffer.
 */
static my_CAN_Frame CAN_ring_buffer[SOFTWARE_CAN_BUFFER_SIZE];

/**
 * @var can_status
 * @brief Current CAN status instance.
 */
static my_CAN_Status can_status = {false, 0, 0, 0};

/**
 * @var sFilterConfig
 * @brief CAN hardware filter configuration structure.
 */
static FDCAN_FilterTypeDef sFilterConfig;

/**
 * @fn my_CAN_Status my_CAN_manual_configuration(uint32_t baudrate)
 * @brief Configure CAN manually using a requested baudrate.
 *
 * @param baudrate
 * @retval Current CAN status instance
 *
 * @details
 * Scans the bit timing table and applies the matching configuration.
 */
my_CAN_Status my_CAN_manual_configuration(uint32_t baudrate) {
	for (int i = 0; i < baudrates_nbr; i++) {
		if (can_timings[i].baudrate != baudrate) {
			continue;
		}
		hfdcan1.Init.NominalPrescaler = can_timings[i].prescaler;
		hfdcan1.Init.NominalTimeSeg1 = can_timings[i].timeSeg1;
		hfdcan1.Init.NominalTimeSeg2 = can_timings[i].timeSeg2;
		return can_status = (my_CAN_Status){true, baudrate, can_status.filter_id, can_status.mask_id};
	}
	return can_status = (my_CAN_Status){false, 0, can_status.filter_id, can_status.mask_id};
}

/**
 * @fn my_CAN_Status my_CAN_auto_configuration(bool to_print)
 * @brief Try all supported baud rates until bus activity is detected.
 *
 * @param to_print If true, "Trying Baud Rate:<baudrate>" messages are printed.
 * 				   If false, the process is silent.
 * @retval Current CAN status instance
 *
 * @detail
 * Applies each bit timing configuration and calls check_FIFO() to check if any traffic is detected.
 * The process is not affected by set filters. It uses global 0x000 filter and mask to capture
 * all possible traffic on the bus.
 */
my_CAN_Status my_CAN_auto_configuration(bool to_print) {
	for (int i = 0; i < baudrates_nbr; i++) {
		if (to_print) my_printf("Trying Baud Rate: %d\r\n", can_timings[i].baudrate);
		hfdcan1.Init.NominalPrescaler = can_timings[i].prescaler;
		hfdcan1.Init.NominalTimeSeg1 = can_timings[i].timeSeg1;
		hfdcan1.Init.NominalTimeSeg2 = can_timings[i].timeSeg2;
		HAL_FDCAN_Init(&hfdcan1);
		HAL_FDCAN_ConfigGlobalFilter(&hfdcan1, FDCAN_ACCEPT_IN_RX_FIFO0, FDCAN_REJECT, FDCAN_REJECT_REMOTE, FDCAN_REJECT_REMOTE);

		if (check_Fifo()) {
			return can_status = (my_CAN_Status){true, can_timings[i].baudrate, can_status.filter_id, can_status.mask_id};
		}
	}
	return can_status = (my_CAN_Status){false, 0, can_status.filter_id, can_status.mask_id};
}

/**
 * @fn static bool check_Fifo(void)
 * @brief Checks for incoming frames to determine if the configured baudrate is valid.
 *
 * @param None
 * @retval true If any CAN frame is detected, else false.
 *
 * @detail
 * Initializes and starts the CAN Sniffer. Waits WAIT_FOR_TRAFFIC time and then checks if
 * any CAN frame is captured.
 *
 * @note
 * For the function to actually detect the CAN baudrate, there should be traffic
 * on the bus. If the traffic is sparse, try increase WAIT_FOR_TRAFFIC delay.
 */
static bool check_Fifo(void) {
	HAL_FDCAN_Init(&hfdcan1);

	HAL_FDCAN_Start(&hfdcan1);
	HAL_Delay(WAIT_FOR_TRAFFIC);
	if (HAL_FDCAN_GetRxFifoFillLevel(&hfdcan1, FDCAN_RX_FIFO0) > 0) {
		HAL_FDCAN_Stop(&hfdcan1);
		return true;
	}
	HAL_FDCAN_Stop(&hfdcan1);
	return false;
}

/**
 * @fn my_CAN_Status my_CAN_set_filter_mask(uint32_t filter_id, uint32_t mask_id)
 * @brief Assign filter and mask values on the structure of current CAN status instance.
 *
 * @param filter_id The desired filter to be set.
 * @param mask_id The desired mask to be set.
 * @retval Current CAN status instance
 */
my_CAN_Status my_CAN_set_filter_mask(uint32_t filter_id, uint32_t mask_id) {
	sFilterConfig.FilterID1 = (filter_id &= 0x7FF);
	sFilterConfig.FilterID2 = (mask_id &= 0x7FF);
	return can_status = (my_CAN_Status){can_status.is_set, can_status.baudrate, filter_id, mask_id};
}

/**
 * @fn my_CAN_Status get_my_CAN_status(bool to_print)
 * @brief Get the current CAN configuration status.
 *
 * @param to_print If true, current CAN status messages are printed.
 * 				   If false, nothing is printed.
 * @retval Current CAN status instance
 */
my_CAN_Status get_my_CAN_status(bool to_print) {
	if (to_print) {
		if (can_status.is_set) {
			my_printf("CAN configured.\r\n");
			my_printf("Baud Rate: %d\r\n", can_status.baudrate);
			my_printf("Filter ID: 0x%03x\r\n", can_status.filter_id);
			my_printf("Mask ID: 0x%03x\r\n", can_status.mask_id);
		} else {
			my_printf("CAN not configured.\r\n");
			my_printf("Baud Rate not set.\r\n");
			my_printf("Filter ID: 0x%03x\r\n", can_status.filter_id);
			my_printf("Mask ID: 0x%03x\r\n", can_status.mask_id);
		}
	}
	return can_status;
}

/**
 * @fn bool my_CAN_start(void)
 * @brief Initialize CAN, configure filters, and start the CAN peripheral.
 *
 * @param None
 * @retval true If CAN is configured(baudrate is set)
 *    	   false If CAN is not configured
 *
 * @detail
 * If CAN is configured, filters are set, CAN peripheral is started and interrupts
 * are enabled.
 */
bool my_CAN_start(void) {
	if (can_status.is_set) {
		HAL_FDCAN_Init(&hfdcan1);

		HAL_FDCAN_ConfigGlobalFilter(&hfdcan1, FDCAN_REJECT, FDCAN_REJECT, FDCAN_REJECT_REMOTE, FDCAN_REJECT_REMOTE);
		sFilterConfig.IdType = FDCAN_STANDARD_ID;
		sFilterConfig.FilterIndex = 0;
		sFilterConfig.FilterType = FDCAN_FILTER_MASK;
		sFilterConfig.FilterConfig = FDCAN_FILTER_TO_RXFIFO0;
		sFilterConfig.FilterID1 = can_status.filter_id;
		sFilterConfig.FilterID2 = can_status.mask_id;
		HAL_FDCAN_ConfigFilter(&hfdcan1, &sFilterConfig);

		HAL_FDCAN_Start(&hfdcan1);
		HAL_FDCAN_ActivateNotification(&hfdcan1, FDCAN_IT_RX_FIFO0_NEW_MESSAGE, 0);
		return true;
	}
	return false;
}

/**
 * @fn void my_CAN_stop(void)
 * @brief Stop CAN activity, disable interrupts, and reset software buffers.
 *
 * @param None
 * @retval None
 */
void my_CAN_stop(void) {
	HAL_FDCAN_Stop(&hfdcan1);
	HAL_FDCAN_DeactivateNotification(&hfdcan1, FDCAN_IT_RX_FIFO0_NEW_MESSAGE);
	head = tail = 0;
}

/**
 * @fn void HAL_FDCAN_RxFifo0Callback(FDCAN_HandleTypeDef *hfdcan, uint32_t RxFifo0ITs)
 * @brief ISR callback for FDCAN RX FIFO0 events.
 *
 * @param hfdcan Pointer to the FDCAN handle that triggered the interrupt (e.g., hfdcan1).
 * @param RxFifo0ITs FIFO0 interrupt flags that indicate which events occurred.
 * @retval None
 *
 * @details
 * This function is called by the HAL library when an RX FIFO0 interrupt occurs.
 * Performs the followings:
 *    - 1. Checks for hardware FIFO overflow: If the `FDCAN_IT_RX_FIFO0_MESSAGE_LOST` flag is set,
 *         the hardware buffer has lost incoming messages. Sets `hardware_CAN_buffer_overflow`
 *         to true and clears the corresponding hardware flag.
 *
 *    - 2. Reads up to 32 frames from FIFO0: Retrieves the message header and data using
 *         `HAL_FDCAN_GetRxMessage` and converts it into a software CAN frame (`my_CAN_Frame`).
 *
 *    - 3. Inserts the frame into the software ring buffer: If the ring buffer is full sets
 *         `software_CAN_buffer_overflow` and the frame is dropped. Otherwise, stores the frame
 *         at the current `head` position and updates `head`.
 */
void HAL_FDCAN_RxFifo0Callback(FDCAN_HandleTypeDef *hfdcan, uint32_t RxFifo0ITs) {
	if (RxFifo0ITs & FDCAN_IT_RX_FIFO0_MESSAGE_LOST) {
		hardware_CAN_buffer_overflow = true;
		__HAL_FDCAN_CLEAR_FLAG(hfdcan, FDCAN_FLAG_RX_FIFO0_MESSAGE_LOST);
	}

	for (int i = 0; i < 32; i++) {
		FDCAN_RxHeaderTypeDef rxHeader;
		uint8_t rxData[8] = {0};
		my_CAN_Frame frame = {0};

		if (HAL_FDCAN_GetRxMessage(hfdcan, FDCAN_RX_FIFO0, &rxHeader, rxData) != HAL_OK) break;
		frame.Identifier = rxHeader.Identifier;
		frame.DataLength = rxHeader.DataLength;
		memcpy(frame.Data, rxData, rxHeader.DataLength);

		uint16_t next_head = (head + 1) & (SOFTWARE_CAN_BUFFER_SIZE - 1);
		if (next_head == tail) {
			software_CAN_buffer_overflow = true;
		} else {
			CAN_ring_buffer[head] = frame;
			head = next_head;
		}
	}
}

/**
 * @fn static bool read_frame_from_software_CAN_buffer(my_CAN_Frame* frame)
 * @brief Pop the next frame from the software buffer.
 *
 * @param frame Pointer to a frame structure where the popped frame will be stored.
 * @retval true If a framed is stored, else false if the ring buffer is empty.
 */
static bool read_frame_from_software_CAN_buffer(my_CAN_Frame* frame) {
    if (head == tail) return false;

    *frame = CAN_ring_buffer[tail];
    tail = (tail + 1) & (SOFTWARE_CAN_BUFFER_SIZE - 1);

    return true;
}

/**
 * @fn void send_frame_over_UART(void)
 * @brief Print all buffered CAN frames over UART.
 *
 * @param None
 * @retval None
 *
 * @details
 * Also prints debug warnings if hardware or software overflow has occurred.
 */
void send_frame_over_UART(void) {
	if (hardware_CAN_buffer_overflow) {
		hardware_CAN_buffer_overflow = false;
		DEBUG_printf("Hardware CAN FIFO overflow!\r\n");
	}

	if (software_CAN_buffer_overflow) {
		software_CAN_buffer_overflow = false;
		DEBUG_printf("Software CAN buffer overflow!\r\n");
	}

	my_CAN_Frame frame;
	while (read_frame_from_software_CAN_buffer(&frame)) {
		my_printf("ID: 0x%03X, DLC: %d, Data:", frame.Identifier, frame.DataLength);
		for (int i = 0; i < frame.DataLength; i++) my_printf(" %02X", frame.Data[i]);
		my_printf("\r\n\n");
	}
}
