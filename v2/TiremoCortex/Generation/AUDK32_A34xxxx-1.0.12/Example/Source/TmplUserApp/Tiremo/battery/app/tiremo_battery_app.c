/**
 *******************************************************************************
 * @file        tiremo_battery_app.c
 * @brief       Tiremo battery application layer implementation
 *
 * @details     AVDD (mV) = (VcoreRef_mV * ADC_FULL_SCALE) / vcoreRaw
 *
 ******************************************************************************/

#include "abov_config.h"
#include "abov_module_config.h"
#include "tiremo_battery_app.h"

#include "../bsp/tiremo_battery_bsp.h"

static bool s_initialized = false;
static uint16_t s_vcoreRaw = 0U;
static uint32_t s_avddMv = 0U;

static void TIREMO_BAT_App_AppendUint(char *pBuf, uint32_t *pPos, uint32_t value)
{
    char digits[10];
    uint32_t count = 0U;
    uint32_t i;

    if (value == 0U)
    {
        pBuf[(*pPos)++] = '0';
        return;
    }

    while (value > 0U)
    {
        digits[count++] = (char)('0' + (value % 10U));
        value /= 10U;
    }

    for (i = count; i > 0U; i--)
    {
        pBuf[(*pPos)++] = digits[i - 1U];
    }
}

static void TIREMO_BAT_App_FormatReading(const TiremoBatReading_t *pReading,
                                         char *pLine,
                                         uint32_t lineSize)
{
    uint32_t pos = 0U;
    uint32_t voltsInt;
    uint32_t voltsDec;

    if ((pReading == NULL) || (pLine == NULL) || (lineSize < 2U))
    {
        return;
    }

    if (!pReading->valid)
    {
        pLine[0] = '\0';
        return;
    }

    voltsInt = pReading->avddMv / 1000U;
    voltsDec = pReading->avddMv % 1000U;

    pLine[pos++] = 'A';
    pLine[pos++] = 'V';
    pLine[pos++] = 'D';
    pLine[pos++] = 'D';
    pLine[pos++] = ':';
    pLine[pos++] = ' ';
    TIREMO_BAT_App_AppendUint(pLine, &pos, voltsInt);
    pLine[pos++] = '.';
    if (voltsDec < 100U)
    {
        pLine[pos++] = '0';
    }
    if (voltsDec < 10U)
    {
        pLine[pos++] = '0';
    }
    TIREMO_BAT_App_AppendUint(pLine, &pos, voltsDec);
    pLine[pos++] = ' ';
    pLine[pos++] = 'V';
    pLine[pos++] = ' ';
    pLine[pos++] = '(';
    pLine[pos++] = 'V';
    pLine[pos++] = 'c';
    pLine[pos++] = 'o';
    pLine[pos++] = 'r';
    pLine[pos++] = 'e';
    pLine[pos++] = ' ';
    pLine[pos++] = 'r';
    pLine[pos++] = 'a';
    pLine[pos++] = 'w';
    pLine[pos++] = ':';
    pLine[pos++] = ' ';
    TIREMO_BAT_App_AppendUint(pLine, &pos, (uint32_t)pReading->vcoreRaw);
    pLine[pos++] = ')';

    if (pos < lineSize)
    {
        pLine[pos] = '\0';
    }
    else
    {
        pLine[lineSize - 1U] = '\0';
    }
}

static bool TIREMO_BAT_App_CalcAvdd(uint16_t vcoreRaw, uint32_t *pAvddMv)
{
    if ((pAvddMv == NULL) || (vcoreRaw == 0U))
    {
        return false;
    }

#if (TIREMO_BAT_VCORE_CALIB_MV > 0U)
    *pAvddMv = ((uint32_t)TIREMO_BAT_VCORE_CALIB_MV * TIREMO_BAT_ADC_FULL_SCALE) / (uint32_t)vcoreRaw;
#else
    *pAvddMv = (TIREMO_BAT_VCORE_REF_MV * TIREMO_BAT_ADC_FULL_SCALE) / (uint32_t)vcoreRaw;
#endif

    return true;
}

bool TIREMO_BAT_App_Init(void)
{
    TIREMO_BAT_BSP_Init();
    s_initialized = true;
    s_vcoreRaw = 0U;
    s_avddMv = 0U;

    return true;
}

bool TIREMO_BAT_App_Read(TiremoBatReading_t *pReading)
{
    uint16_t vcoreRaw = 0U;
    uint32_t avddMv = 0U;

    if (!s_initialized || (pReading == NULL))
    {
        return false;
    }

    if (!TIREMO_BAT_BSP_ReadVcore(&vcoreRaw))
    {
        pReading->vcoreRaw = 0U;
        pReading->avddMv = 0U;
        pReading->valid = false;
        return false;
    }

    if (!TIREMO_BAT_App_CalcAvdd(vcoreRaw, &avddMv))
    {
        pReading->vcoreRaw = vcoreRaw;
        pReading->avddMv = 0U;
        pReading->valid = false;
        return false;
    }

    s_vcoreRaw = vcoreRaw;
    s_avddMv = avddMv;

    pReading->vcoreRaw = vcoreRaw;
    pReading->avddMv = avddMv;
    pReading->valid = true;

    return true;
}

bool TIREMO_BAT_App_Update(void)
{
    TiremoBatReading_t reading;
    char line[48];

    if (!TIREMO_BAT_App_Read(&reading))
    {
        TIREMO_BAT_App_OnDebugOutput("AVDD: Read Error");
        return false;
    }

    TIREMO_BAT_App_FormatReading(&reading, line, sizeof(line));
    TIREMO_BAT_App_OnDebugOutput(line);
    return true;
}

uint32_t TIREMO_BAT_App_GetAvddMv(void)
{
    return s_avddMv;
}

uint16_t TIREMO_BAT_App_GetVcoreRaw(void)
{
    return s_vcoreRaw;
}
__attribute__((weak)) void TIREMO_BAT_App_OnDebugOutput(const char *line)
{
    (void)line;
}

