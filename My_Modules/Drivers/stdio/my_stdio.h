/**
 * @file my_stdio.h
 * @brief Lightweight printf/scanf on top of UART.
 *
 * @details
 * Provides simplified printf/scanf equivalents that redirect I/O
 * through USART3 via the my_uart module.
 */

#ifndef MY_STDIO_H
#define MY_STDIO_H

#include <stdarg.h>
#include <stdio.h>
#include "my_uart.h"

/**
 * @fn int my_printf(const char* fmt, ...)
 * @brief Formatted UART output similar to printf.
 *
 * @param fmt Format string (printf style).
 * @param ... Arguments referenced by the format specifiers.
 * @retval Number of characters written (excluding null terminator).
 *
 * @details
 * Formats input into a small local buffer and
 * transmits using UART.
 */
int my_printf(const char* fmt, ...);

/**
 * @fn int my_scanf(const char* fmt, ...)
 * @brief Formatted UART input similar to scanf.
 *
 * @param fmt Format string (scanf style).
 * @param ... Pointers where parsed values will be stored.
 * @retval Number of successfully parsed input items.
 *
 * @details
 * Reads characters one by one via UART until newline or
 * carriage and parses them.
 */
int my_scanf(const char* fmt, ...);

#endif /* MY_STDIO_H */
