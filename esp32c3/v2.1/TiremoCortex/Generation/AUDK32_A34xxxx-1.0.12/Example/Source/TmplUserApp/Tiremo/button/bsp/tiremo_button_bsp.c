/**
 *******************************************************************************
 * @file        tiremo_button_bsp.c
 * @brief       Tiremo button board support package (BSP) implementation
 *
 * Copyright 2024 Tiremo. All rights reserved.
 ******************************************************************************/

#include "abov_config.h"
#include "abov_module_config.h"
#include "tiremo_button_bsp.h"
#include "tiremo_button_board.h"

static const TiremoBtnPin_t s_btnPinMap[TIREMO_BTN_COUNT] =
{
    { TIREMO_BOARD_USER_BTN_PORT, TIREMO_BOARD_USER_BTN_PIN }
};

static TiremoBtnIntrCallback_t s_intrCallback = NULL;
static void *s_intrCallbackContext = NULL;

static volatile uint8_t s_intrEventPending[TIREMO_BTN_COUNT];
static volatile TiremoBtnState_e s_intrLastState[TIREMO_BTN_COUNT];

static bool s_portIrqRegistered = false;

static bool TIREMO_BTN_IsValidId(TiremoBtnId_e btnId)
{
    return ((uint32_t)btnId < TIREMO_BTN_COUNT);
}

static uint32_t TIREMO_BTN_GetIntrMask(TiremoBtnId_e btnId)
{
    return TIREMO_BTN_INTR_MASK(s_btnPinMap[btnId].pin);
}

static void TIREMO_BTN_Intr_ProcessEvent(uint32_t un32Event)
{
    uint32_t index;

    for (index = 0U; index < TIREMO_BTN_COUNT; index++)
    {
        if ((un32Event & TIREMO_BTN_GetIntrMask((TiremoBtnId_e)index)) == 0U)
        {
            continue;
        }

        s_intrLastState[index] = TIREMO_BTN_GetState((TiremoBtnId_e)index);
        s_intrEventPending[index] = 1U;

        if (s_intrCallback != NULL)
        {
            s_intrCallback((TiremoBtnId_e)index, s_intrLastState[index], s_intrCallbackContext);
        }
    }
}

static void TIREMO_BTN_Intr_PortHandler(uint32_t un32Event, void *pContext)
{
    (void)pContext;

    TIREMO_BTN_Intr_ProcessEvent(un32Event);
}

void TIREMO_BTN_Init(void)
{
    uint32_t index;

    for (index = 0U; index < TIREMO_BTN_COUNT; index++)
    {
        s_intrEventPending[index] = 0U;
        s_intrLastState[index] = TIREMO_BTN_STATE_RELEASED;
    }
}

PCU_PORT_e TIREMO_BTN_ReadRaw(TiremoBtnId_e btnId)
{
    PCU_PORT_e level = PCU_PORT_HIGH;

    if (TIREMO_BTN_IsValidId(btnId))
    {
        const TiremoBtnPin_t *pin = &s_btnPinMap[btnId];

        HAL_PCU_GetInputValue(pin->port, pin->pin, &level);
    }

    return level;
}

bool TIREMO_BTN_IsPressed(TiremoBtnId_e btnId)
{
    /* Active-low: LOW level means pressed. */
    return (TIREMO_BTN_ReadRaw(btnId) == PCU_PORT_LOW);
}

bool TIREMO_BTN_IsReleased(TiremoBtnId_e btnId)
{
    return (TIREMO_BTN_ReadRaw(btnId) == PCU_PORT_HIGH);
}

TiremoBtnState_e TIREMO_BTN_GetState(TiremoBtnId_e btnId)
{
    if (TIREMO_BTN_IsPressed(btnId))
    {
        return TIREMO_BTN_STATE_PRESSED;
    }

    return TIREMO_BTN_STATE_RELEASED;
}

void TIREMO_BTN_Intr_Init(TiremoBtnId_e btnId)
{
    const TiremoBtnPin_t *pin;
    PCU_IRQ_CFG_t irqCfg;

    if (!TIREMO_BTN_IsValidId(btnId))
    {
        return;
    }

    pin = &s_btnPinMap[btnId];

    (void)HAL_PCU_SetIntrPort(pin->port, pin->pin,
                              PCU_INTR_MODE_EDGE,
                              PCU_INTR_TRG_BOTH_LEVEL_EDGE,
                              0U);

    if (!s_portIrqRegistered)
    {
        irqCfg.eId         = pin->port;
        irqCfg.eOps        = PCU_OPS_INTR;
        irqCfg.pfnHandler  = TIREMO_BTN_Intr_PortHandler;
        irqCfg.pContext    = NULL;
        irqCfg.un32IRQPrio = TIREMO_BOARD_USER_BTN_IRQ_PRIO;
        irqCfg.un8IntNum   = 0U;
        (void)HAL_PCU_SetIRQ(&irqCfg);
        s_portIrqRegistered = true;
    }
}

void TIREMO_BTN_Intr_Enable(TiremoBtnId_e btnId)
{
    if (TIREMO_BTN_IsValidId(btnId))
    {
        const TiremoBtnPin_t *pin = &s_btnPinMap[btnId];

        (void)HAL_PCU_SetIntrPort(pin->port, pin->pin,
                                  PCU_INTR_MODE_EDGE,
                                  PCU_INTR_TRG_BOTH_LEVEL_EDGE,
                                  0U);
    }
}

void TIREMO_BTN_Intr_Disable(TiremoBtnId_e btnId)
{
    if (TIREMO_BTN_IsValidId(btnId))
    {
        const TiremoBtnPin_t *pin = &s_btnPinMap[btnId];

        (void)HAL_PCU_SetIntrPort(pin->port, pin->pin,
                                  PCU_INTR_MODE_DISABLE,
                                  PCU_INTR_TRG_DISABLE,
                                  0U);
    }
}

void TIREMO_BTN_Intr_SetCallback(TiremoBtnIntrCallback_t pfnCallback, void *pContext)
{
    s_intrCallback = pfnCallback;
    s_intrCallbackContext = pContext;
}

bool TIREMO_BTN_Intr_IsEventPending(TiremoBtnId_e btnId)
{
    if (TIREMO_BTN_IsValidId(btnId))
    {
        return (s_intrEventPending[btnId] != 0U);
    }

    return false;
}

void TIREMO_BTN_Intr_ClearEvent(TiremoBtnId_e btnId)
{
    if (TIREMO_BTN_IsValidId(btnId))
    {
        s_intrEventPending[btnId] = 0U;
    }
}

TiremoBtnState_e TIREMO_BTN_Intr_GetLastState(TiremoBtnId_e btnId)
{
    if (TIREMO_BTN_IsValidId(btnId))
    {
        return s_intrLastState[btnId];
    }

    return TIREMO_BTN_STATE_RELEASED;
}
