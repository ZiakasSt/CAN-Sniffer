/**
 * @file my_uart.c
 * @brief UART helper function implementations.
 *
 * @details
 * Implements my_uart_transmit_buffer() and my_uart_receive_char()
 * for sending and receiving data over USART3 (huart3).
 * These functions are blocking and use HAL_MAX_DELAY.
 */

#include "my_uart.h"


/**
 * @fn void my_uart_transmit_buffer(const char* buf)
 * @brief Transmit a null-terminated string buffer over USART3.
 *
 * @param buf Pointer to a null-terminated string to transmit.
 * @retval None
 *
 * @details
 * Uses HAL_UART_Transmit with huart3. Blocks until the entire string
 * is transmitted. Converts the input string to uint8_t* for HAL compatibility.
 */
void my_uart_transmit_buffer(const char* buf) {
	HAL_UART_Transmit(&huart3, (uint8_t*)buf, strlen(buf), HAL_MAX_DELAY);
}

/**
 * @fn void my_uart_receive_char(char* ch)
 * @brief Receive a single character from USART3.
 *
 * @param ch Pointer to a char variable where the received character will be stored.
 * @retval None
 *
 * @details
 * Uses HAL_UART_Receive with huart3 to read a single byte.
 * Blocks until a character is received.
 */
void my_uart_receive_char(char* ch) {
	HAL_UART_Receive(&huart3, (uint8_t*)ch, 1, HAL_MAX_DELAY);
}
