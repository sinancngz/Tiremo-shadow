/**
 *******************************************************************************
 * @file        tiremo_button_bsp.h
 * @brief       Tiremo button board support package (BSP) API
 *
 * @details     Hardware abstraction layer for on-board push buttons.
 *
 *              Two usage modes are supported:
 *              - Polling : TIREMO_BTN_IsPressed(), TIREMO_BTN_GetState()
 *              - Interrupt : TIREMO_BTN_Intr_Init(), TIREMO_BTN_Intr_Enable()
 *
 *              Polling example:
 *              @code
 *              TIREMO_BTN_Init();
 *              if (TIREMO_BTN_IsPressed(TIREMO_BTN_USER)) { ... }
 *              @endcode
 *
 *              Interrupt example:
 *              @code
 *              TIREMO_BTN_Init();
 *              TIREMO_BTN_Intr_Init(TIREMO_BTN_USER);
 *              TIREMO_BTN_Intr_Enable(TIREMO_BTN_USER);
 *              @endcode
 *
 * @note        Invalid btnId values return safe defaults (released / HIGH).
 * @note        GPIO direction and pull configuration are handled by PRV_PORT_Init().
 *
 * Copyright 2024 Tiremo. All rights reserved.
 ******************************************************************************/

#ifndef TIREMO_BUTTON_BSP_H_
#define TIREMO_BUTTON_BSP_H_

#include <stdbool.h>
#include <stdint.h>

#include "tiremo_button_board.h"
#include "hal_pcu.h"

/**
 * @brief Logical button identifier.
 * @note  Values map 1:1 to the board pin table in tiremo_button_board.h.
 */
typedef enum
{
    TIREMO_BTN_USER = 0U
} TiremoBtnId_e;

/**
 * @brief Logical button state.
 */
typedef enum
{
    TIREMO_BTN_STATE_RELEASED = 0U,
    TIREMO_BTN_STATE_PRESSED
} TiremoBtnState_e;

/**
 * @brief Interrupt callback invoked from the PCU IRQ handler.
 * @param[in] btnId   Button that generated the interrupt.
 * @param[in] state   Current logical state after the edge event.
 * @param[in] pContext User context pointer supplied at registration.
 * @warning Keep this callback short; it runs in interrupt context.
 */
typedef void (*TiremoBtnIntrCallback_t)(TiremoBtnId_e btnId, TiremoBtnState_e state, void *pContext);

/**
 * @brief   Initialize the button BSP layer (polling mode).
 */
void TIREMO_BTN_Init(void);

/**
 * @brief       Read the raw PCU input level of a button.
 * @param[in]   btnId  Target button identifier.
 * @return      PCU_PORT_LOW if pin is low, PCU_PORT_HIGH if high or invalid btnId.
 */
PCU_PORT_e TIREMO_BTN_ReadRaw(TiremoBtnId_e btnId);

/**
 * @brief       Check whether a button is currently pressed.
 * @param[in]   btnId  Target button identifier.
 * @return      true if pressed, false if released or btnId is invalid.
 */
bool TIREMO_BTN_IsPressed(TiremoBtnId_e btnId);

/**
 * @brief       Check whether a button is currently released.
 * @param[in]   btnId  Target button identifier.
 * @return      true if released, false if pressed or btnId is invalid.
 */
bool TIREMO_BTN_IsReleased(TiremoBtnId_e btnId);

/**
 * @brief       Get the logical pressed/released state of a button.
 * @param[in]   btnId  Target button identifier.
 * @return      TIREMO_BTN_STATE_PRESSED or TIREMO_BTN_STATE_RELEASED.
 */
TiremoBtnState_e TIREMO_BTN_GetState(TiremoBtnId_e btnId);

/**
 * @brief       Configure external interrupt for a button pin.
 * @param[in]   btnId  Target button identifier.
 * @details     Sets edge-triggered interrupt (both edges) and registers the
 *              port IRQ handler. Safe to call once per button at startup.
 */
void TIREMO_BTN_Intr_Init(TiremoBtnId_e btnId);

/**
 * @brief       Enable the external interrupt for a button.
 * @param[in]   btnId  Target button identifier.
 */
void TIREMO_BTN_Intr_Enable(TiremoBtnId_e btnId);

/**
 * @brief       Disable the external interrupt for a button.
 * @param[in]   btnId  Target button identifier.
 */
void TIREMO_BTN_Intr_Disable(TiremoBtnId_e btnId);

/**
 * @brief       Register a callback invoked on every button interrupt.
 * @param[in]   pfnCallback  Callback function (NULL to unregister).
 * @param[in]   pContext     User context passed to the callback.
 */
void TIREMO_BTN_Intr_SetCallback(TiremoBtnIntrCallback_t pfnCallback, void *pContext);

/**
 * @brief       Check whether an interrupt event is pending for a button.
 * @param[in]   btnId  Target button identifier.
 * @return      true if an unprocessed interrupt event exists.
 */
bool TIREMO_BTN_Intr_IsEventPending(TiremoBtnId_e btnId);

/**
 * @brief       Clear the pending interrupt event flag for a button.
 * @param[in]   btnId  Target button identifier.
 */
void TIREMO_BTN_Intr_ClearEvent(TiremoBtnId_e btnId);

/**
 * @brief       Get the last state captured in the interrupt handler.
 * @param[in]   btnId  Target button identifier.
 * @return      Last state recorded by the ISR, or RELEASED if invalid btnId.
 */
TiremoBtnState_e TIREMO_BTN_Intr_GetLastState(TiremoBtnId_e btnId);

#endif /* TIREMO_BUTTON_BSP_H_ */
