#include "abov_config.h"
#include "abov_module_config.h"

#include "tiremo_esp32_app.h"
#include "../bsp/tiremo_esp32_bsp.h"
#include "../bsp/tiremo_esp32_board.h"
#include "../../DebugLibrary/debug_framework.h"

static bool s_tiremoEsp32AppReady = false;

static bool TIREMO_ESP32_App_TryAtOnce(char *response, uint16_t respSize)
{
    if ((response == NULL) || (respSize < 2U))
    {
        return false;
    }

    return TIREMO_ESP32_BSP_SendAtCommand("AT", response, respSize);
}

bool TIREMO_ESP32_App_Init(void)
{
    if (!TIREMO_ESP32_BSP_Init())
    {
        return false;
    }

    s_tiremoEsp32AppReady = true;
    return true;
}

bool TIREMO_ESP32_App_RunAtTest(void)
{
    char response[64];
    uint8_t attempt;

    if (!s_tiremoEsp32AppReady)
    {
        return false;
    }

    for (attempt = 0U; attempt < TIREMO_ESP32_AT_MAX_ATTEMPTS; attempt++)
    {
        if (TIREMO_ESP32_App_TryAtOnce(response, (uint16_t)sizeof(response)))
        {
            return true;
        }
    }

    if (response[0] != '\0')
    {
        DebugFramework_Printf("[AT] last rsp: [%s]\r\n", response);
    }
    else
    {
        DebugFramework_PutsLine("[AT] last rsp: (empty)");
    }

    return false;
}

bool TIREMO_ESP32_App_RunAtTestWithRecovery(void)
{
    uint8_t cycle;

    if (!s_tiremoEsp32AppReady)
    {
        return false;
    }

    for (cycle = 0U; cycle <= TIREMO_ESP32_AT_POWER_CYCLE_MAX; cycle++)
    {
        if (TIREMO_ESP32_App_RunAtTest())
        {
            return true;
        }

        if (cycle < TIREMO_ESP32_AT_POWER_CYCLE_MAX)
        {
            DebugFramework_Printf("[AT] Power cycle %u/%u\r\n",
                                  (unsigned)(cycle + 1U),
                                  (unsigned)TIREMO_ESP32_AT_POWER_CYCLE_MAX);
            TIREMO_ESP32_BSP_PowerCycle();
        }
    }

    return false;
}
