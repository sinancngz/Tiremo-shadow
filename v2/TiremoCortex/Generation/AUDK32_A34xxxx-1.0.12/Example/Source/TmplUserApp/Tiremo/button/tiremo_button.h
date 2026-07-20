/**
 *******************************************************************************
 * @file        tiremo_button.h
 * @brief       Tiremo button library umbrella header
 *
 * @details     Single entry point for the Tiremo button library.
 *
 *              Layer structure:
 *              - BSP  : Hardware read + external interrupt control
 *              - APP  : Polling patterns and interrupt event helpers
 *
 *              Polling usage (GPIO_Input):
 *              @code
 *              TIREMO_SysTick_Init();
 *              TIREMO_BTN_Init();
 *              TIREMO_BTN_App_Init();
 *              edge = TIREMO_BTN_App_GetEdge(TIREMO_BTN_USER);
 *              @endcode
 *
 *              Interrupt usage (GPIO_EINT):
 *              @code
 *              TIREMO_BTN_Init();
 *              TIREMO_BTN_App_Intr_Init(TIREMO_BTN_USER);
 *              TIREMO_BTN_App_Intr_RegisterHandler(MyHandler, NULL);
 *              @endcode
 *
 * Copyright 2024 Tiremo. All rights reserved.
 ******************************************************************************/

#ifndef TIREMO_BUTTON_H_
#define TIREMO_BUTTON_H_

#include "bsp/tiremo_button_bsp.h"
#include "app/tiremo_button_app.h"

#endif /* TIREMO_BUTTON_H_ */
