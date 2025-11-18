/**
 * @file my_uart.h
 * @brief UART helper functions for transmitting and receiving data.
 *
 * @details
 * Provides simple wrappers around STM32 HAL UART functions to:
 *   - Transmit a null-terminated string buffer
 *   - Receive a single character
 *
 * Uses USART3 (huart3) as the communication interface.
 */

#ifndef MY_USART_H
#define MY_USART_H

#include <string.h>
#include "stm32h7xx.h"

/**
 * @var huart3
 * @brief Global USART3 handle.
 *
 * @details
 * This handle is generated and initialized in main.c
 * by the STM32CubeMX .ioc configuration file. It configures
 * the USART3 peripheral used by all functions in this module.
 *
 * Typical parameters:
 *    - Mode: Asynchronous
 *    - Baud Rate: 921600
 *    - Word Length: 8 bits
 *    - Parity: None
 *    - Stop Bits: 1
 */
extern UART_HandleTypeDef huart3;

/**
 * @fn void my_uart_transmit_buffer(const char* buf)
 * @brief Transmit a null-terminated string buffer over USART3.
 *
 * @param buf Pointer to a null-terminated string to transmit.
 * @retval None
 *
 * @details
 * This function wraps HAL_UART_Transmit, sending the full string and
 * blocking until all characters are transmitted. Uses HAL_MAX_DELAY
 * for blocking transmission.
 */
void my_uart_transmit_buffer(const char* buf);

/**
 * @fn void my_uart_receive_char(char* ch)
 * @brief Receive a single character from USART3.
 *
 * @param ch Pointer to a char variable where the received character will be stored.
 * @retval None
 *
 * @details
 * This function wraps HAL_UART_Receive, receiving one character at a time
 * and blocking until a character is received. Uses HAL_MAX_DELAY.
 */
void my_uart_receive_char(char* ch);

#endif /* MY_USART_H */
