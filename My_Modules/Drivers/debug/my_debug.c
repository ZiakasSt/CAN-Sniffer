/**
 * @file my_debug.c
 * @brief Implementation of DWT-based cycle measurement and debug printing.
 *
 * @details
 * Provides functions for starting and reading the Cortex-M DWT cycle counter,
 * along with a framed debug print utility that outputs using UART.
 */

#include "my_debug.h"

/**
 * @fn uint32_t my_DWT_GetCycles_start(void)
 * @brief Start a DWT cycle count measurement.
 *
 * @param None
 * @retval Starting cycle counter value.
 *
 * @details
 * Enables the trace unit, resets the CYCCNT register,
 * starts the cycle counter and returns its value.
 */
uint32_t my_DWT_GetCycles_start(void) {
	/* Enable trace capability */
  CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;

  /* Disable counter before reset */
  DWT->CTRL &= ~DWT_CTRL_CYCCNTENA_Msk;

  /* Reset cycle register */
  DWT->CYCCNT = 0;

  /* Enable counter */
  DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;

  return DWT->CYCCNT;
}

/**
 * @fn uint32_t my_DWT_GetCycles_end(void)
 * @brief Read the current DWT cycle counter.
 *
 * @param None
 * @retval Current cycle counter value.
 *
 * @details
 * Returns the current value of the CYCCNT register.
 */
uint32_t my_DWT_GetCycles_end(void) {
  return DWT->CYCCNT;
}

/**
 * @fn void DEBUG_printf(const char* fmt, ...)
 * @brief Print a framed debug message via UART.
 *
 * @param fmt Format string (printf style).
 * @param ... Arguments referenced by the format specifiers.
 * @retval None
 *
 * @details
 * Formats the message, then prints it between two debug banners:
 *    $$$$$$$$$ DEBUG print START $$$$$$$$$
 *    $$$$$$$$$ DEBUG print END $$$$$$$$$$$
 *
 * Intended for visually distinct diagnostic logs.
 */
void DEBUG_printf(const char* fmt, ...) {
	char buf[128];
	va_list args;
	va_start(args, fmt);
	vsnprintf(buf, sizeof(buf), fmt, args);
	va_end(args);

	my_printf("\n\n\n$$$$$$$$$ DEBUG print START $$$$$$$$$\r\n");
	my_uart_transmit_buffer(buf);
	my_printf("$$$$$$$$$ DEBUG print END $$$$$$$$$$$\r\n\n\n");
}
