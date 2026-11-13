#ifndef LAB_BOARD_H
#define LAB_BOARD_H

/* Active-low button on PA0, active-high external LED on PA1. */
#define BUTTON_PIN 0U
#define LED_PIN 1U

/* Set to 1, rebuild the selected target, and start from reset for the fault lab. */
#ifndef LAB_SKIP_GPIO_CLOCK
#define LAB_SKIP_GPIO_CLOCK 0
#endif

#endif
