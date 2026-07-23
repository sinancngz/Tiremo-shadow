/**
 *******************************************************************************
 * @file        tiremo_button_app.c
 * @brief       Tiremo button application layer implementation
 *
 * Copyright 2024 Tiremo. All rights reserved.
 ******************************************************************************/

#include "abov_config.h"
#include "abov_module_config.h"
#include "tiremo_button_app.h"

#include "../../common/tiremo_systick.h"

#define TIREMO_BTN_APP_POLL_MS      (10U)

static TiremoBtnState_e s_prevBtnState[TIREMO_BTN_COUNT];

static TiremoBtnEdge_e TIREMO_BTN_App_StateToEdge(TiremoBtnState_e state)
{
    if (state == TIREMO_BTN_STATE_PRESSED)
    {
        return TIREMO_BTN_EDGE_PRESSED;
    }

    return TIREMO_BTN_EDGE_RELEASED;
}

void TIREMO_BTN_App_Init(void)
{
    uint32_t index;

    for (index = 0U; index < TIREMO_BTN_COUNT; index++)
    {
        s_prevBtnState[index] = TIREMO_BTN_GetState((TiremoBtnId_e)index);
    }
}

TiremoBtnEdge_e TIREMO_BTN_App_GetEdge(TiremoBtnId_e btnId)
{
    TiremoBtnState_e currentState;
    TiremoBtnEdge_e edge = TIREMO_BTN_EDGE_NONE;

    if ((uint32_t)btnId >= TIREMO_BTN_COUNT)
    {
        return TIREMO_BTN_EDGE_NONE;
    }

    currentState = TIREMO_BTN_GetState(btnId);

    if (currentState != s_prevBtnState[btnId])
    {
        edge = TIREMO_BTN_App_StateToEdge(currentState);
        s_prevBtnState[btnId] = currentState;
    }

    return edge;
}

void TIREMO_BTN_App_WaitForPress(TiremoBtnId_e btnId)
{
    while (!TIREMO_BTN_IsPressed(btnId))
    {
        TIREMO_SysTick_DelayMs(TIREMO_BTN_APP_POLL_MS);
    }
}

void TIREMO_BTN_App_WaitForRelease(TiremoBtnId_e btnId)
{
    while (!TIREMO_BTN_IsReleased(btnId))
    {
        TIREMO_SysTick_DelayMs(TIREMO_BTN_APP_POLL_MS);
    }
}

bool TIREMO_BTN_App_WaitForPressTimeout(TiremoBtnId_e btnId, uint32_t timeoutMs)
{
    uint32_t elapsedMs = 0U;

    while (!TIREMO_BTN_IsPressed(btnId))
    {
        if (elapsedMs >= timeoutMs)
        {
            return false;
        }

        TIREMO_SysTick_DelayMs(TIREMO_BTN_APP_POLL_MS);
        elapsedMs += TIREMO_BTN_APP_POLL_MS;
    }

    return true;
}

bool TIREMO_BTN_App_WaitForReleaseTimeout(TiremoBtnId_e btnId, uint32_t timeoutMs)
{
    uint32_t elapsedMs = 0U;

    while (!TIREMO_BTN_IsReleased(btnId))
    {
        if (elapsedMs >= timeoutMs)
        {
            return false;
        }

        TIREMO_SysTick_DelayMs(TIREMO_BTN_APP_POLL_MS);
        elapsedMs += TIREMO_BTN_APP_POLL_MS;
    }

    return true;
}

void TIREMO_BTN_App_Intr_Init(TiremoBtnId_e btnId)
{
    TIREMO_BTN_Intr_Init(btnId);
    TIREMO_BTN_Intr_Enable(btnId);
}

void TIREMO_BTN_App_Intr_RegisterHandler(TiremoBtnIntrCallback_t pfnCallback, void *pContext)
{
    TIREMO_BTN_Intr_SetCallback(pfnCallback, pContext);
}

TiremoBtnEdge_e TIREMO_BTN_App_Intr_GetEvent(TiremoBtnId_e btnId)
{
    TiremoBtnEdge_e edge = TIREMO_BTN_EDGE_NONE;

    if (!TIREMO_BTN_Intr_IsEventPending(btnId))
    {
        return TIREMO_BTN_EDGE_NONE;
    }

    edge = TIREMO_BTN_App_StateToEdge(TIREMO_BTN_Intr_GetLastState(btnId));
    TIREMO_BTN_Intr_ClearEvent(btnId);

    return edge;
}

bool TIREMO_BTN_App_Intr_IsEventPending(TiremoBtnId_e btnId)
{
    return TIREMO_BTN_Intr_IsEventPending(btnId);
}
