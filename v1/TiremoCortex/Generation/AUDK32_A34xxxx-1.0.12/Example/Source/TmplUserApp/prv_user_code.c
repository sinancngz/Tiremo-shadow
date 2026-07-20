/**
 *******************************************************************************
 * @file        prv_user_code.c
 * @brief       TiremoCortex user entry — delegates to Tiremo_Process
 *
 * Copyright 2024 ABOV Semiconductor Co.,Ltd. All rights reserved.
 ******************************************************************************/

#include "abov_config.h"
#include "abov_module_config.h"

#include <string.h>

#include "hal_uart.h"

#include "Tiremo_Process/tiremo_app.h"
#include "Tiremo/esp32/tiremo_esp32.h"
#include "Tiremo/esp32/bsp/tiremo_esp32_board.h"
#include "Tiremo/common/tiremo_systick.h"
#include "Tiremo/DebugLibrary/debug_framework.h"

/**********************************************************************
 * Local AT helper — only for MAC debug in this file. Does NOT change BSP.
 **********************************************************************/
static uint8_t MacAt_SendAndCollect(const char *cmd, char *response, uint16_t respSize)
{
    uint8_t tx[48];
    uint32_t cmdLen;
    uint32_t got = 0U;
    uint32_t t0;
    uint8_t byte;

    if ((cmd == NULL) || (response == NULL) || (respSize < 2U))
    {
        return 0U;
    }

    cmdLen = (uint32_t)strlen(cmd);
    if ((cmdLen + 2U) > sizeof(tx))
    {
        return 0U;
    }

    memcpy(tx, cmd, cmdLen);
    tx[cmdLen] = (uint8_t)'\r';
    tx[cmdLen + 1U] = (uint8_t)'\n';
    memset(response, 0, respSize);

    (void)HAL_UART_Transmit(TIREMO_ESP32_UART_ID, tx, cmdLen + 2U, true);

    t0 = TIREMO_SysTick_GetMs();
    while (got < ((uint32_t)respSize - 1U))
    {
        if ((TIREMO_SysTick_GetMs() - t0) >= 500U)
        {
            break;
        }

        if (HAL_UART_Receive(TIREMO_ESP32_UART_ID, &byte, 1U, true) == HAL_ERR_OK)
        {
            response[got++] = (char)byte;
            response[got] = '\0';
            t0 = TIREMO_SysTick_GetMs();

            if (strstr(response, "OK") != NULL)
            {
                break;
            }
            if (strstr(response, "ERROR") != NULL)
            {
                break;
            }
        }
    }

    return (strstr(response, "OK") != NULL) ? 1U : 0U;
}

/**********************************************************************
 * @brief       Read ESP32 STA MAC (local helper only — BSP untouched)
 **********************************************************************/
static void PrintEsp32Mac(void)
{
    char response[96];
    char *p;
    char *end;

    DebugFramework_PutsLine("\n\r[MAC] --- start ---");

    if (!TIREMO_ESP32_App_Init())
    {
        DebugFramework_PutsLine("[MAC] ESP32 power/init FAILED");
        return;
    }
    DebugFramework_PutsLine("[MAC] ESP32 power OK");

    if (!TIREMO_ESP32_App_RunAtTestWithRecovery())
    {
        DebugFramework_PutsLine("[MAC] AT test FAILED (ESP32 not answering)");
        return;
    }
    DebugFramework_PutsLine("[MAC] AT OK — sending AT+CIPSTAMAC?");

    if (MacAt_SendAndCollect("AT+CIPSTAMAC?", response, (uint16_t)sizeof(response)) == 0U)
    {
        DebugFramework_PutsLine("[MAC] CIPSTAMAC failed — raw:");
        DebugFramework_PutsLine(response[0] != '\0' ? response : "(empty)");
        return;
    }

    DebugFramework_PutsLine("[MAC] raw:");
    DebugFramework_PutsLine(response);

    p = strstr(response, "+CIPSTAMAC:\"");
    if (p != NULL)
    {
        p += 12;
        end = strchr(p, '"');
        if (end != NULL)
        {
            *end = '\0';
            DebugFramework_PutsLine("[MAC] STA MAC:");
            DebugFramework_PutsLine(p);
            return;
        }
    }

    DebugFramework_PutsLine("[MAC] parse failed (no +CIPSTAMAC in response)");
}

/**********************************************************************
 * @brief       User application entry (called from main after HAL init)
 **********************************************************************/
void PRV_USER_Code(void)
{
    TiremoApp_Init();
    //PrintEsp32Mac();
    TiremoApp_Run();
}
