/**
 * @file my_stdio.c
 * @brief Implementation of lightweight printf/scanf over UART.
 *
 * @details
 * Implements printf/scanf-style formatted input and output,
 * routed through USART3 via the my_uart module.
 */

#include "my_stdio.h"

/**
 * @fn int my_printf(const char* fmt, ...)
 * @brief Formatted UART output similar to printf.
 *
 * @param fmt Format string (printf style).
 * @param ... Arguments referenced by the format specifiers.
 * @retval Number of characters written (excluding null terminator).
 *
 * @details
 * Formats the input into a small stack buffer using vsnprintf,
 * then transmits it through my_uart_transmit_buffer().
 */
int my_printf(const char* fmt, ...) {
	char buf[128];
	va_list args;
	va_start(args, fmt);
	int res = vsnprintf(buf, sizeof(buf), fmt, args);
	va_end(args);

	my_uart_transmit_buffer(buf);

	return res;
}

/**
 * @fn int my_scanf(const char* fmt, ...)
 * @brief Formatted UART input similar to scanf.
 *
 * @param fmt Format string (scanf style).
 * @param ... Pointers where parsed values will be stored.
 * @retval Number of successfully parsed input items.
 *
 * @details
 * Blocks until a newline or carriage return is received.
 * Accumulates characters into a temporary buffer and parses them
 * using vsscanf(). Input is obtained byte by byte via my_uart_receive_char().
 */
int my_scanf(const char* fmt, ...) {
	char buf[128];
	int i = 0;
	char ch;

	while (i < (sizeof(buf) - 1)) {
		my_uart_receive_char(&ch);

		if (ch == '\r' || ch == '\n') {
			break;
		}

		buf[i++] = ch;
	}

	buf[i] = '\0';

	va_list args;
	va_start(args, fmt);
	int res = vsscanf(buf, fmt, args);
	va_end(args);

	return res;
}
