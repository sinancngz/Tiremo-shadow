/**
 *******************************************************************************
 * @file        tiremo_battery_bsp.c
 * @brief       Tiremo battery BSP implementation
 *
 * @details     HAL_ADC read path for VCORE (channel 23).
 *              ADC init: user_adc.c. Runtime: SetSeqConfig → Start → delay → GetData.
 *
 ******************************************************************************/

#include "abov_config.h"
#include "abov_module_config.h"
#include "tiremo_battery_bsp.h"

#include "hal_adc.h"

static bool s_bspReady = false;

void TIREMO_BAT_BSP_Init(void)
{
    /* Wait for internal VCORE reference to stabilize after PRV_ADC_Init(). */
    for (volatile uint32_t i = 0U; i < TIREMO_BAT_VCORE_STABILIZE_LOOPS; i++)
    {
    }

    s_bspReady = true;
}

bool TIREMO_BAT_BSP_ReadChannel(uint8_t channel, uint16_t *pRaw)
{
    HAL_ERR_e eErr;
    ADC_SEQ_DATA_t tAdcData;
    ADC_SEQ_TRG_CFG_t tSeqCfg;

    if (!s_bspReady || (pRaw == NULL))
    {
        return false;
    }

    tSeqCfg.eType = ADC_TRG_TYPE_INDEPENDENT;
    tSeqCfg.eTrgSrc = ADC_TRG_SRC_ADST;
    tSeqCfg.un8TrgNum = 0U;
    tSeqCfg.utCfg.tInd.un8SeqNum = 0U;
    tSeqCfg.utCfg.tInd.un8ChNum = channel;
    tSeqCfg.utCfg.tInd.un8SamplingTime = TIREMO_BAT_ADC_SAMPLING_TIME;

    eErr = HAL_ADC_SetSeqConfig(TIREMO_BAT_ADC_ID, &tSeqCfg);
    if (eErr != HAL_ERR_OK)
    {
        return false;
    }

    eErr = HAL_ADC_Start(TIREMO_BAT_ADC_ID);
    if (eErr != HAL_ERR_OK)
    {
        return false;
    }

    for (volatile uint32_t i = 0U; i < TIREMO_BAT_ADC_CONV_DELAY_LOOPS; i++)
    {
    }

    tAdcData.bReadDDR = false;
    eErr = HAL_ADC_GetData(TIREMO_BAT_ADC_ID, 0U, &tAdcData);
    if (eErr != HAL_ERR_OK)
    {
        (void)HAL_ADC_Stop(TIREMO_BAT_ADC_ID);
        return false;
    }

    *pRaw = tAdcData.un16Result;
    (void)HAL_ADC_Stop(TIREMO_BAT_ADC_ID);

    return true;
}

bool TIREMO_BAT_BSP_ReadVcore(uint16_t *pRaw)
{
    return TIREMO_BAT_BSP_ReadChannel(TIREMO_BAT_VCORE_CHANNEL, pRaw);
}
