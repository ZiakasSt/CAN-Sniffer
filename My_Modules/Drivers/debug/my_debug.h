/**
 * @file my_debug.h
 * @brief Debug utilities for cycle measurement and formatted debug output.
 *
 * @details
 * Provides:
 *   - Simple DWT-based cycle counter start/end helpers
 *   - DEBUG_printf() for tagged debug messages over UART
 */

#ifndef MY_DEBUG_H
#define MY_DEBUG_H

#include "my_stdio.h"

/**
 * @fn uint32_t my_DWT_GetCycles_start(void)
 * @brief Start a DWT cycle count measurement.
 *
 * @param None
 * @retval Starting cycle counter value.
 *
 * @details
 * Initializes and enables the Cortex-M DWT CYCCNT counter.
 * Returns the initial value of CYCCNT (normally zero).
 * Call this before my_DWT_GetCycles_end() to measure elapsed CPU cycles.
 */
uint32_t my_DWT_GetCycles_start(void);

/**
 * @fn uint32_t my_DWT_GetCycles_end(void)
 * @brief Read the current DWT cycle counter.
 *
 * @param None
 * @retval Current cycle counter value.
 *
 * @details
 * Returns the current value of the CYCCNT register.
 * Use together with my_DWT_GetCycles_start() to compute the elapsed cycles.
 */
uint32_t my_DWT_GetCycles_end(void);

/**
 * @fn void DEBUG_printf(const char* fmt, ...)
 * @brief Print a formatted debug message with framing markers.
 *
 * @param fmt Format string (printf style).
 * @param ... Arguments referenced by the format specifiers.
 * @retval None
 *
 * @details
 * Outputs a message wrapped between "DEBUG print" banners
 * through USART3 via the my_uart module.
 */
void DEBUG_printf(const char* fmt, ...);

#endif /* MY_DEBUG_H */
