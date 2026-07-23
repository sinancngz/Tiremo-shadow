/**
 * @file    user_button.h
 * @brief   PC9 user button — init (NVIC), interrupt press detection, LED feedback
 *
 * Pin GPIO / debounce / SetIntrPort: UserButton_Init() in user_button.c
 * Pin map and active level: config/board_config.h
 * NVIC + handler registration: UserButton_Init() in user_button.c
 * ISR entry point: PCU_IRQHandler_PCU_ID_C in user_button_isr.c
 */

#ifndef USER_BUTTON_H
#define USER_BUTTON_H

#include <stdint.h>

void UserButton_Init(void);
void UserButton_OnInterrupt(uint32_t un32Event);

/** 1 ms tick — call from SysTick for long-press timing while button is held. */
void UserButton_Tick1ms(void);

/** Returns 1 once per short press (< APP_BTN_LONG_PRESS_MS), then clears. */
uint8_t UserButton_ConsumeShortPress(void);

/** Returns 1 once when held >= APP_BTN_LONG_PRESS_MS, then clears. */
uint8_t UserButton_ConsumeLongPress(void);

/** 1 while button is physically pressed. */
uint8_t UserButton_IsPressed(void);

#endif /* USER_BUTTON_H */
