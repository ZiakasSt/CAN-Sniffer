/**
 * @file settings_menu.h
 * @brief CAN sniffer settings menu interface.
 *
 * @details
 * Provides:
 *   - System state enumeration (STATE_MENU, STATE_RUN)
 *   - Global system_state variable
 *   - settings_menu() function to configure CAN
 *
 * This header declares the API for configuring the
 * CAN sniffer via a User menu.
 */

#ifndef SETTINGS_MENU_H
#define SETTINGS_MENU_H

#include "my_debug.h"
#include "my_can.h"

/**
 * @enum SystemState
 * @brief Global operating mode of the CAN sniffer.
 *
 * @details
 * STATE_MENU:
 *     CAN sniffer is paused. The settings menu is active.
 *     The system remains here until the user configures CAN and chooses
 *     to start the sniffer.
 *
 * STATE_RUN:
 *     CAN sniffer is active. Frames are being captured and forwarded
 *     over UART. No menu interaction occurs in this state.
 */
typedef enum {
    STATE_RUN,
    STATE_MENU
} SystemState;

/**
 * @var system_state
 * @brief Global state variable controlling the mode of the system.
 *
 * @details
 * Shared between main(), settings_menu(), and the EXTI interrupt callback.
 * Marked volatile because it may be modified asynchronously from the
 * User Button (PC13) EXTI interrupt. The main loop polls this variable
 * to decide whether to run the sniffer or enter the menu.
 */
extern volatile SystemState system_state;

/**
 * @fn void settings_menu(void)
 * @brief Blocking menu used to configure the CAN sniffer.
 *
 * @param None
 * @retval None
 *
 * @details
 * Behavior:
 *   - Stops CAN on entry to prevent traffic while configuring.
 *   - Prints a menu and waits for user input.
 *   - Allows:
 *       - Auto/manual baud rate configuration
 *       - Filter/mask setup
 *       - Querying CAN status
 *       - Starting the CAN sniffer.
 *   - Runs an internal infinite loop and returns only when:
 *         1) the user selects 'q', and
 *         2) CAN has been successfully configured.
 *
 * @note This function is blocking and will not return until the CAN sniffer
 * is properly configured and started.
 */
void settings_menu(void);

#endif /* SETTINGS_MENU_H */
