/**
 * @file settings_menu.c
 * @brief CAN sniffer settings menu implementation.
 *
 * @details
 * Implements settings_menu() that calls functions for:
 *   - Auto/manual CAN baud rate configuration
 *   - Filter and mask setup
 *   - Querying CAN status
 *   - Starting the CAN sniffer
 *
 * The menu is blocking and returns only when:
 *     1) the user selects 'q', and
 *     2) CAN has been successfully configured.
 *
 * The system_state variable controls whether the main loop is in menu or
 * run mode. PC13 (User Button) can asynchronously switch to menu mode.
 */

#include "settings_menu.h"

/**
 * @var system_state
 * @brief Global state variable controlling the system mode.
 *
 * @details
 * Initialized to STATE_MENU to start in the settings menu.
 * Shared between main(), settings_menu(), and the EXTI interrupt callback.
 * Marked volatile because it may be modified asynchronously from the
 * User Button (PC13) EXTI interrupt. The main loop polls this variable
 * to decide whether to run the sniffer or enter the menu.
 */
volatile SystemState system_state = STATE_MENU;


/**
 * @fn static void print_menu(void)
 * @brief Prints the settings menu options over UART.
 *
 * @param None
 * @retval None
 *
 * @details
 * Static helper function used internally by settings_menu().
 * Displays available options for configuring the CAN sniffer:
 *   - a: Auto Configure CAN Baud Rate
 *   - m: Manual Configure CAN Baud Rate
 *   - s: Set CAN Filter-Mask
 *   - g: Get CAN Sniffer status
 *   - q: Quit and Start CAN Sniffer
 */
static void print_menu(void) {
	my_printf("*************************************\r\n");
	my_printf("* CAN Sniffer - Settings menu       *\r\n");
	my_printf("*                                   *\r\n");
	my_printf("* a: Auto Configure CAN Baud Rate   *\r\n");
	my_printf("* m: Manual Configure CAN Baud Rate *\r\n");
	my_printf("* s: Set CAN Filter-Mask            *\r\n");
	my_printf("* g: Get CAN Sniffer status         *\r\n");
	my_printf("* q: Quit and Start CAN Sniffer     *\r\n");
	my_printf("*************************************\r\n\n");
}

/**
 * @fn void settings_menu(void)
 * @brief Blocking menu to configure the CAN sniffer.
 *
 * @param None
 * @retval None
 *
 * @details
 * Stops CAN on entry to prevent traffic while configuring. Prints options
 * menu and waits for user input to configure the CAN sniffer.
 *
 * Supports: The above options that are printed using print_menu().
 *
 * Runs an internal infinite loop and returns only when:
 *     1) the user selects 'q', and
 *     2) CAN has been successfully configured.
 *
 * @note This function is blocking and will not return until the CAN sniffer
 * is properly configured and started.
 */
void settings_menu(void) {
	/* Stop CAN communication to prevent traffic while configuring. */
	my_CAN_stop();

	/* Show options menu */
	print_menu();

	/* Main menu loop */
	while(1) {
		/* Flush UART to remove any leftover input */
		__HAL_UART_FLUSH_DRREGISTER(&huart3);

		/* Read a single character option from the user */
		char option = '\0';
		my_scanf(" %c", &option);

		switch (option) {
			case 'a':
				/* Auto-configure CAN baud rate */
				if (my_CAN_auto_configuration(true).is_set) {
					my_printf("\nCAN Detected!\r\n\n");
					(void) get_my_CAN_status(true); // Show current CAN status
					my_printf("\n");
				} else {
					my_printf("\nNo CAN Detected!\r\n\n");
				}
				my_printf("\n");
				print_menu(); // Show menu again
				break;
			case 'm':
				/* Manual CAN baud rate configuration */
				uint32_t baudrate = 0;
				my_printf("Provide one of the supported Baud Rates:\r\n");
				for (int i = 0; i < baudrates_nbr; i++){
					my_printf("%d\r\n", can_timings[i].baudrate);
				}
				my_printf("\n");
				my_scanf(" %d", &baudrate);
				if (my_CAN_manual_configuration(baudrate).is_set) {
					(void) get_my_CAN_status(true);
					my_printf("\n");
				} else {
					my_printf("Configuration failed.\r\n\n");
				}
				my_printf("\n");
				print_menu();
				break;
			case 's':
				/* Set CAN filter and mask */
				uint32_t filter_id = 0;
				uint32_t mask_id = 0;
				my_printf("Provide filter in 0x<filter_id> format\r\n");
				my_scanf(" 0x%x", &filter_id);
				my_printf("\n");
				my_printf("Provide mask in 0x<mask_id> format\r\n");
				my_scanf(" 0x%x", &mask_id);
				my_printf("\n\n");
				(void) my_CAN_set_filter_mask(filter_id, mask_id);
				get_my_CAN_status(true);
				my_printf("\n\n");
				print_menu();
				break;
			case 'g':
				/* Query CAN status */
				(void) get_my_CAN_status(true);
				my_printf("\n");
				print_menu();
				break;
			case 'q':
				/* Attempt to start CAN sniffer */
				if (my_CAN_start()) {
					system_state = STATE_RUN; // Change global state
					return; // Exit menu
				}
				my_printf("CAN not configured.\r\n\n");
				my_printf("\n");
				print_menu();
				break;
			default:
				/* Invalid option entered */
				my_printf("Option not found. Try again...\r\n\n");
				my_printf("\n");
				print_menu();
		}
	}
}
