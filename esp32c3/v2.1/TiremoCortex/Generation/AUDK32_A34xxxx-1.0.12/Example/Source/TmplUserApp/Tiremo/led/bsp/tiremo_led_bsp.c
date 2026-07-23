/**
 *******************************************************************************
 * @file        tiremo_led_bsp.c
 * @brief       Tiremo LED board support package (BSP) implementation
 *
 * Copyright 2024 Tiremo. All rights reserved.
 ******************************************************************************/

#include "abov_config.h"
#include "abov_module_config.h"
#include "tiremo_led_bsp.h"
#include "tiremo_led_board.h"

#include "hal_pcu.h"

/** Software shadow of each LED state (used for toggle and get-state). */
static bool s_ledState[TIREMO_LED_COUNT];

static const TiremoLedPin_t s_ledPinMap[TIREMO_LED_COUNT] =
{
    { TIREMO_BOARD_LED1_PORT,  TIREMO_BOARD_LED1_PIN  },
    { TIREMO_BOARD_LED2_PORT,  TIREMO_BOARD_LED2_PIN  },
    { TIREMO_BOARD_LED3_PORT,  TIREMO_BOARD_LED3_PIN  },
    { TIREMO_BOARD_LED4_PORT,  TIREMO_BOARD_LED4_PIN  },
    { TIREMO_BOARD_LED5_PORT,  TIREMO_BOARD_LED5_PIN  },
    { TIREMO_BOARD_LED6_PORT,  TIREMO_BOARD_LED6_PIN  },
    { TIREMO_BOARD_LED7_PORT,  TIREMO_BOARD_LED7_PIN  },
    { TIREMO_BOARD_LED8_PORT,  TIREMO_BOARD_LED8_PIN  },
    { TIREMO_BOARD_LED9_PORT,  TIREMO_BOARD_LED9_PIN  },
    { TIREMO_BOARD_LED10_PORT, TIREMO_BOARD_LED10_PIN }
};

static bool TIREMO_LED_IsValidId(TiremoLedId_e ledId)
{
    return ((uint32_t)ledId < TIREMO_LED_COUNT);
}

static void TIREMO_LED_WritePin(TiremoLedId_e ledId, bool isOn)
{
    const TiremoLedPin_t *pin = &s_ledPinMap[ledId];

    /* Active-low: CLEAR = on, SET = off. */
    if (isOn)
    {
        HAL_PCU_SetOutputBit(pin->port, pin->pin, PCU_OUTPUT_BIT_CLEAR);
    }
    else
    {
        HAL_PCU_SetOutputBit(pin->port, pin->pin, PCU_OUTPUT_BIT_SET);
    }

    s_ledState[ledId] = isOn;
}

void TIREMO_LED_Init(void)
{
    uint32_t index;

    for (index = 0U; index < TIREMO_LED_COUNT; index++)
    {
        TIREMO_LED_WritePin((TiremoLedId_e)index, false);
    }
}

void TIREMO_LED_On(TiremoLedId_e ledId)
{
    if (TIREMO_LED_IsValidId(ledId))
    {
        TIREMO_LED_WritePin(ledId, true);
    }
}

void TIREMO_LED_Off(TiremoLedId_e ledId)
{
    if (TIREMO_LED_IsValidId(ledId))
    {
        TIREMO_LED_WritePin(ledId, false);
    }
}

void TIREMO_LED_Toggle(TiremoLedId_e ledId)
{
    if (TIREMO_LED_IsValidId(ledId))
    {
        TIREMO_LED_WritePin(ledId, !s_ledState[ledId]);
    }
}

void TIREMO_LED_SetState(TiremoLedId_e ledId, bool isOn)
{
    if (TIREMO_LED_IsValidId(ledId))
    {
        TIREMO_LED_WritePin(ledId, isOn);
    }
}

bool TIREMO_LED_GetState(TiremoLedId_e ledId)
{
    if (TIREMO_LED_IsValidId(ledId))
    {
        return s_ledState[ledId];
    }

    return false;
}

void TIREMO_LED_AllOn(void)
{
    uint32_t index;

    for (index = 0U; index < TIREMO_LED_COUNT; index++)
    {
        TIREMO_LED_WritePin((TiremoLedId_e)index, true);
    }
}

void TIREMO_LED_AllOff(void)
{
    uint32_t index;

    for (index = 0U; index < TIREMO_LED_COUNT; index++)
    {
        TIREMO_LED_WritePin((TiremoLedId_e)index, false);
    }
}
