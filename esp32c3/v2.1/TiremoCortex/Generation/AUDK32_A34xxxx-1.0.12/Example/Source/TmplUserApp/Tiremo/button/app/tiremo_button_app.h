/**
 *******************************************************************************
 * @file        tiremo_button_app.h
 * @brief       Tiremo button application layer API
 *
 * @details     High-level button patterns built on top of the BSP layer.
 *
 *              Two usage modes:
 *              - Polling  : TIREMO_BTN_App_GetEdge(), TIREMO_BTN_App_WaitForPress()
 *              - Interrupt: TIREMO_BTN_App_Intr_Init(), TIREMO_BTN_App_Intr_GetEvent()
 *
 *              Polling prerequisites:
 *              - TIREMO_SysTick_Init() before wait functions.
 *              - TIREMO_BTN_Init() and TIREMO_BTN_App_Init() at startup.
 *
 *              Interrupt prerequisites:
 *              - TIREMO_BTN_Init() and TIREMO_BTN_App_Intr_Init() at startup.
 *              - Optionally register TIREMO_BTN_App_Intr_RegisterHandler() for
 *                immediate ISR-side response.
 *
 * @note        Polling: call TIREMO_BTN_App_GetEdge() periodically (e.g. 50 ms).
 * @note        Interrupt: call TIREMO_BTN_App_Intr_GetEvent() from the main loop
 *              or handle events inside the registered callback.
 *
 * Copyright 2024 Tiremo. All rights reserved.
 ******************************************************************************/

#ifndef TIREMO_BUTTON_APP_H_
#define TIREMO_BUTTON_APP_H_

#include <stdbool.h>
#include <stdint.h>

#include "../bsp/tiremo_button_bsp.h"

/**
 * @brief Button edge event detected since the previous poll or interrupt.
 */
typedef enum
{
    TIREMO_BTN_EDGE_NONE = 0U,
    TIREMO_BTN_EDGE_PRESSED,
    TIREMO_BTN_EDGE_RELEASED
} TiremoBtnEdge_e;

/* -------------------------------------------------------------------------- */
/* Polling mode                                                               */
/* -------------------------------------------------------------------------- */

/**
 * @brief   Initialize polling-mode edge detection state.
 * @details Samples current button state as baseline to avoid spurious edges.
 */
void TIREMO_BTN_App_Init(void);

/**
 * @brief       Detect a press or release edge since the last poll call.
 * @param[in]   btnId  Target button identifier.
 * @return      Edge type, or TIREMO_BTN_EDGE_NONE if unchanged.
 */
TiremoBtnEdge_e TIREMO_BTN_App_GetEdge(TiremoBtnId_e btnId);

/**
 * @brief       Block until the button is pressed.
 * @param[in]   btnId  Target button identifier.
 */
void TIREMO_BTN_App_WaitForPress(TiremoBtnId_e btnId);

/**
 * @brief       Block until the button is released.
 * @param[in]   btnId  Target button identifier.
 */
void TIREMO_BTN_App_WaitForRelease(TiremoBtnId_e btnId);

/**
 * @brief       Block until pressed or timeout expires.
 * @param[in]   btnId      Target button identifier.
 * @param[in]   timeoutMs  Maximum wait time in milliseconds.
 * @return      true if pressed before timeout, false on timeout.
 */
bool TIREMO_BTN_App_WaitForPressTimeout(TiremoBtnId_e btnId, uint32_t timeoutMs);

/**
 * @brief       Block until released or timeout expires.
 * @param[in]   btnId      Target button identifier.
 * @param[in]   timeoutMs  Maximum wait time in milliseconds.
 * @return      true if released before timeout, false on timeout.
 */
bool TIREMO_BTN_App_WaitForReleaseTimeout(TiremoBtnId_e btnId, uint32_t timeoutMs);

/* -------------------------------------------------------------------------- */
/* Interrupt mode                                                             */
/* -------------------------------------------------------------------------- */

/**
 * @brief       Initialize interrupt mode for a button.
 * @param[in]   btnId  Target button identifier.
 * @details     Configures PCU edge interrupt and enables it.
 */
void TIREMO_BTN_App_Intr_Init(TiremoBtnId_e btnId);

/**
 * @brief       Register an interrupt callback for immediate ISR-side handling.
 * @param[in]   pfnCallback  Callback function (NULL to unregister).
 * @param[in]   pContext     User context passed to the callback.
 */
void TIREMO_BTN_App_Intr_RegisterHandler(TiremoBtnIntrCallback_t pfnCallback, void *pContext);

/**
 * @brief       Consume a pending interrupt event from the main loop.
 * @param[in]   btnId  Target button identifier.
 * @return      Press or release edge, or TIREMO_BTN_EDGE_NONE if no event.
 * @details     Clears the pending flag when an event is returned.
 */
TiremoBtnEdge_e TIREMO_BTN_App_Intr_GetEvent(TiremoBtnId_e btnId);

/**
 * @brief       Check whether an interrupt event is waiting in the main loop.
 * @param[in]   btnId  Target button identifier.
 * @return      true if TIREMO_BTN_App_Intr_GetEvent() would return an edge.
 */
bool TIREMO_BTN_App_Intr_IsEventPending(TiremoBtnId_e btnId);

#endif /* TIREMO_BUTTON_APP_H_ */
