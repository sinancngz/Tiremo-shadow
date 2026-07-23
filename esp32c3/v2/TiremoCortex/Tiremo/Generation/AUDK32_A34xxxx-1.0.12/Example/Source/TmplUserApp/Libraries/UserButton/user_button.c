/**
 * @file    user_button.c
 * @brief   PC9 user button — interrupt-driven press detection and LED action
 */

#include "abov_config.h"
#include "abov_module_config.h"
#include "../../config/board_config.h"
#include "../../config/app_config.h"
#include "hal_pcu.h"
#include "user_button.h"

extern void LED_ON(PCU_ID_e port, PCU_PIN_ID_e pin);
extern void LED_OFF(PCU_ID_e port, PCU_PIN_ID_e pin);
extern void PCU_IRQHandler_PCU_ID_C(uint32_t un32Event, void *pContext);
extern uint32_t MqttPort_ABOV_GetTickMs(void);

#define USER_BTN_INTR_MASK      (0x3UL << ((uint32_t)BOARD_USER_BTN_PIN * 2U))

static uint8_t s_btnLedApplied = 0xFFU;
static volatile uint8_t s_btnPressed = 0U;
static volatile uint8_t s_longFired = 0U;
static volatile uint8_t s_shortPending = 0U;
static volatile uint8_t s_longPending = 0U;
static volatile uint32_t s_pressStartMs = 0U;

typedef struct
{
    PCU_ID_e port;
    PCU_PIN_ID_e pin;
} LedPin_t;

static const LedPin_t s_allLeds[] =
{
    {BOARD_LED1_PORT,  BOARD_LED1_PIN},
    {BOARD_LED2_PORT,  BOARD_LED2_PIN},
    {BOARD_LED3_PORT,  BOARD_LED3_PIN},
    {BOARD_LED4_PORT,  BOARD_LED4_PIN},
    {BOARD_LED5_PORT,  BOARD_LED5_PIN},
    {BOARD_LED6_PORT,  BOARD_LED6_PIN},
    {BOARD_LED7_PORT,  BOARD_LED7_PIN},
    {BOARD_LED8_PORT,  BOARD_LED8_PIN},
    {BOARD_LED9_PORT,  BOARD_LED9_PIN},
    {BOARD_LED10_PORT, BOARD_LED10_PIN},
};

static uint8_t UserButton_ReadPressed(void)
{
    PCU_PORT_e level = PCU_PORT_HIGH;

    if (HAL_PCU_GetInputValue(BOARD_USER_BTN_PORT, BOARD_USER_BTN_PIN, &level) != HAL_ERR_OK)
        return 0U;

    return (level == BOARD_USER_BTN_ACTIVE_LEVEL) ? 1U : 0U;
}

static void UserButton_SetAllLeds(uint8_t on)
{
    for (uint32_t i = 0U; i < (sizeof(s_allLeds) / sizeof(s_allLeds[0])); i++)
    {
        if (on != 0U)
            LED_ON(s_allLeds[i].port, s_allLeds[i].pin);
        else
            LED_OFF(s_allLeds[i].port, s_allLeds[i].pin);
    }
}

static void UserButton_UpdateLeds(void)
{
    uint8_t wantOn = UserButton_ReadPressed();

    if (wantOn == s_btnLedApplied)
        return;

    s_btnLedApplied = wantOn;
    UserButton_SetAllLeds(wantOn);
}

static void UserButton_HandleEdge(void)
{
    uint8_t isPressed = UserButton_ReadPressed();
    uint32_t now = MqttPort_ABOV_GetTickMs();

    UserButton_UpdateLeds();

    if (isPressed != 0U)
    {
        s_btnPressed = 1U;
        s_longFired = 0U;
        s_pressStartMs = now;
        return;
    }

    if (s_btnPressed == 0U)
        return;

    if ((s_longFired == 0U) && ((now - s_pressStartMs) < APP_BTN_LONG_PRESS_MS))
        s_shortPending = 1U;

    s_btnPressed = 0U;
}

void UserButton_OnInterrupt(uint32_t un32Event)
{
    if ((un32Event & USER_BTN_INTR_MASK) == 0U)
        return;

    UserButton_HandleEdge();
}

void UserButton_Tick1ms(void)
{
    uint32_t now;

    if ((s_btnPressed == 0U) || (s_longFired != 0U))
        return;

    now = MqttPort_ABOV_GetTickMs();
    if ((now - s_pressStartMs) >= APP_BTN_LONG_PRESS_MS)
    {
        s_longFired = 1U;
        s_longPending = 1U;
    }
}

void UserButton_Init(void)
{
    PCU_IRQ_CFG_t irqCfg;
    PCU_DEBOUNCE_CLK_CFG_t debounceClk;
    debounceClk.eMccr      = PCU_CLK_MCCR_MCLK;
    debounceClk.un8MccrDiv = 10U;
    (void)HAL_PCU_SetClkDebounce(BOARD_USER_BTN_PORT, &debounceClk);
    (void)HAL_PCU_SetPortDebounce(BOARD_USER_BTN_PORT, BOARD_USER_BTN_PIN, true);

    (void)HAL_PCU_SetIntrPort(BOARD_USER_BTN_PORT, BOARD_USER_BTN_PIN,
                              PCU_INTR_MODE_EDGE,
                              PCU_INTR_TRG_BOTH_LEVEL_EDGE,
                              0U);

    irqCfg.eId         = BOARD_USER_BTN_PORT;
    irqCfg.eOps        = PCU_OPS_INTR;
    irqCfg.pfnHandler  = PCU_IRQHandler_PCU_ID_C;
    irqCfg.pContext    = NULL;
    irqCfg.un32IRQPrio = BOARD_USER_BTN_IRQ_PRIO;
    irqCfg.un8IntNum   = 0U;
    (void)HAL_PCU_SetIRQ(&irqCfg);

    s_btnLedApplied = 0xFFU;
    s_btnPressed = 0U;
    s_longFired = 0U;
    s_shortPending = 0U;
    s_longPending = 0U;
    s_pressStartMs = 0U;
    UserButton_UpdateLeds();
}

uint8_t UserButton_ConsumeShortPress(void)
{
    if (s_shortPending == 0U)
        return 0U;

    s_shortPending = 0U;
    return 1U;
}

uint8_t UserButton_ConsumeLongPress(void)
{
    if (s_longPending == 0U)
        return 0U;

    s_longPending = 0U;
    return 1U;
}

uint8_t UserButton_IsPressed(void)
{
    return UserButton_ReadPressed();
}
