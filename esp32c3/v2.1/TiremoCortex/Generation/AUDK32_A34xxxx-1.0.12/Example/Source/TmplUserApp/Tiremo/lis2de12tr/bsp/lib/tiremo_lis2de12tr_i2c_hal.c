#include "abov_config.h"
#include "abov_module_config.h"
#include "tiremo_lis2de12tr_i2c_hal.h"

#include "../../../../../../../Platform/HAL/I2C/hal_i2c_prv.h"

#ifndef True
#define True true
#endif

#ifndef I2C_POLL_WAIT_TIMEOUT
#define I2C_POLL_WAIT_TIMEOUT   (1000000UL)
#endif

#define I2C_OPSTATUS_TIMEOUT    (0xFFFFFFFFUL)
#define I2C_TRANSACTION_GUARD   (I2C_POLL_WAIT_TIMEOUT)
#define I2C_FAIL_RECOVER_LIMIT  (2U)

static uint8_t s_i2cFailStreak = 0U;

static uint32_t tiremo_lis2de12tr_i2c_get_op_status(I2C_Type* ptI2c)
{
    uint32_t un32Status = 0U;
    uint32_t un32Timeout = I2C_POLL_WAIT_TIMEOUT;

    while (!GET_I2C_IER_FLAG(ptI2c))
    {
        if (un32Timeout == 0U)
        {
            return I2C_OPSTATUS_TIMEOUT;
        }
        un32Timeout--;
    }

    un32Status = GET_I2C_SR(ptI2c);
    return un32Status;
}

static void tiremo_lis2de12tr_i2c_set_op_stop(I2C_Type* ptI2c)
{
    uint32_t un32Timeout = I2C_POLL_WAIT_TIMEOUT;

    SET_I2C_CR_STOP(ptI2c, true);

    while (!GET_I2C_IER_FLAG(ptI2c))
    {
        if (un32Timeout == 0U)
        {
            break;
        }
        un32Timeout--;
    }

    SET_I2C_CR_STOP(ptI2c, false);
}

static void tiremo_lis2de12tr_i2c_flush_flags(I2C_Type* ptI2c)
{
    uint32_t un32Timeout = 10000U;

    while ((GET_I2C_IER_FLAG(ptI2c) != 0U) && (un32Timeout > 0U))
    {
        SET_I2C_SR_CLEAR(ptI2c);
        un32Timeout--;
    }
}

static void tiremo_lis2de12tr_i2c_bus_begin(I2C_ID_e eId, I2C_Type* ptI2c)
{
    IRQn_Type eIrq = I2C_GetIRQNum((P_I2C_ID_e)eId);

    NVIC_DisableIRQ(eIrq);
    SET_I2C_IER_EN(ptI2c, false);
    tiremo_lis2de12tr_i2c_flush_flags(ptI2c);
}

static void tiremo_lis2de12tr_i2c_bus_end(I2C_ID_e eId, I2C_Type* ptI2c)
{
    IRQn_Type eIrq = I2C_GetIRQNum((P_I2C_ID_e)eId);

    tiremo_lis2de12tr_i2c_flush_flags(ptI2c);
    SET_I2C_IER_EN(ptI2c, false);
    NVIC_ClearPendingIRQ(eIrq);
    NVIC_DisableIRQ(eIrq);
}

static void tiremo_lis2de12tr_i2c_apply_config(I2C_ID_e eId)
{
    I2C_CFG_t tCfg =
    {
        .eMode = I2C_MODE_MASTER,
        .uPeriod.tFreq.un32Freq = 100000,
        .tSdht.bEnable = True,
        .tSdht.un16Hold = 50,
    };

    (void)HAL_I2C_SetConfig(eId, &tCfg);
    (void)HAL_I2C_SetIRQ(eId, I2C_OPS_POLL, NULL, NULL, 3U);
}

void tiremo_lis2de12tr_i2c_prepare_bus(I2C_ID_e eId)
{
    I2C_Type* ptI2c;
    uint32_t index;

    if ((uint32_t)eId >= I2C_CH_NUM)
    {
        return;
    }

    ptI2c = I2C_GetReg((P_I2C_ID_e)eId);
    tiremo_lis2de12tr_i2c_bus_begin(eId, ptI2c);

    for (index = 0U; index < 9U; index++)
    {
        tiremo_lis2de12tr_i2c_set_op_stop(ptI2c);
        SET_I2C_SR_CLEAR(ptI2c);
    }

    tiremo_lis2de12tr_i2c_bus_end(eId, ptI2c);
}

void tiremo_lis2de12tr_i2c_full_recover(I2C_ID_e eId)
{
    if ((uint32_t)eId >= I2C_CH_NUM)
    {
        return;
    }

    tiremo_lis2de12tr_i2c_prepare_bus(eId);
    (void)HAL_I2C_Uninit(eId);
    (void)HAL_I2C_Init(eId);
    tiremo_lis2de12tr_i2c_apply_config(eId);
    s_i2cFailStreak = 0U;
}

static void tiremo_lis2de12tr_i2c_on_error(I2C_ID_e eId)
{
    s_i2cFailStreak++;

    if (s_i2cFailStreak >= I2C_FAIL_RECOVER_LIMIT)
    {
        tiremo_lis2de12tr_i2c_full_recover(eId);
    }
    else
    {
        tiremo_lis2de12tr_i2c_prepare_bus(eId);
    }
}

HAL_ERR_e tiremo_lis2de12tr_i2c_write_read(I2C_ID_e eId, uint8_t un8SlaveAddr,
                                           uint8_t* pun8Out, uint32_t un32WrLen,
                                           uint8_t* pun8In, uint32_t un32RdLen)
{
    I2C_Type* ptI2c;
    uint32_t un32Status;
    uint32_t un32WrCnt = 0U;
    uint32_t un32RdCnt = 0U;
    uint8_t un8Phase = 0U;
    uint32_t un32Guard = I2C_TRANSACTION_GUARD;
    HAL_ERR_e eResult = HAL_ERR_HW;
    bool bBusLocked = false;

    if ((uint32_t)eId >= I2C_CH_NUM)
    {
        return HAL_ERR_INVALID_ID;
    }

    if ((un32WrLen == 0U) && (un32RdLen == 0U))
    {
        return HAL_ERR_OK;
    }

    if ((un32RdLen > 0U) && (pun8In == NULL))
    {
        return HAL_ERR_PARAMETER;
    }

    if ((un32WrLen > 0U) && (pun8Out == NULL))
    {
        return HAL_ERR_PARAMETER;
    }

    ptI2c = I2C_GetReg((P_I2C_ID_e)eId);
    tiremo_lis2de12tr_i2c_bus_begin(eId, ptI2c);
    bBusLocked = true;

    SET_I2C_CR_ACK(ptI2c, true);
    SET_I2C_DR_TX(ptI2c, (uint8_t)(un8SlaveAddr << 1));
    SET_I2C_CR_START(ptI2c, true);

    while (un32Guard-- > 0U)
    {
        un32Status = tiremo_lis2de12tr_i2c_get_op_status(ptI2c);

        if (un32Status == I2C_OPSTATUS_TIMEOUT)
        {
            break;
        }

        if (un8Phase == 0U)
        {
            switch (un32Status)
            {
                case I2C_MASTER_TX_ADDR_ACK:
                    if (un32WrCnt < un32WrLen)
                    {
                        SET_I2C_DR_TX(ptI2c, pun8Out[un32WrCnt++]);
                        SET_I2C_SR_CLEAR(ptI2c);
                    }
                    else if (un32RdLen > 0U)
                    {
                        un8Phase = 1U;
                        SET_I2C_CR_ACK(ptI2c, true);
                        SET_I2C_DR_TX(ptI2c, (uint8_t)((un8SlaveAddr << 1) | 0x01U));
                        SET_I2C_CR_START(ptI2c, true);
                        SET_I2C_SR_CLEAR(ptI2c);
                    }
                    else
                    {
                        eResult = HAL_ERR_OK;
                        goto done;
                    }
                    break;

                case I2C_TX_DATA_ACK:
                    if (un32WrCnt < un32WrLen)
                    {
                        SET_I2C_DR_TX(ptI2c, pun8Out[un32WrCnt++]);
                        SET_I2C_SR_CLEAR(ptI2c);
                    }
                    else if (un32RdLen > 0U)
                    {
                        un8Phase = 1U;
                        SET_I2C_CR_ACK(ptI2c, true);
                        SET_I2C_DR_TX(ptI2c, (uint8_t)((un8SlaveAddr << 1) | 0x01U));
                        SET_I2C_CR_START(ptI2c, true);
                        SET_I2C_SR_CLEAR(ptI2c);
                    }
                    else
                    {
                        eResult = HAL_ERR_OK;
                        goto done;
                    }
                    break;

                default:
                    goto done;
            }
        }
        else
        {
            switch (un32Status)
            {
                case I2C_MASTER_RX_ADDR_ACK:
                    SET_I2C_SR_CLEAR(ptI2c);
                    break;

                case I2C_RX_DATA_ACK:
                {
                    uint8_t rxByte = 0U;

                    if (un32RdCnt >= un32RdLen)
                    {
                        eResult = HAL_ERR_OK;
                        goto done;
                    }

                    rxByte = GET_I2C_DR_RX(ptI2c);

                    if (un32RdCnt < un32RdLen)
                    {
                        pun8In[un32RdCnt++] = rxByte;
                    }

                    if (un32RdCnt == un32RdLen)
                    {
                        SET_I2C_CR_ACK(ptI2c, false);
                    }
                    else
                    {
                        SET_I2C_SR_CLEAR(ptI2c);
                    }
                    break;
                }

                case I2C_MASTER_RX_DATA_NOACK:
                    if (un32RdCnt == un32RdLen)
                    {
                        eResult = HAL_ERR_OK;
                    }
                    goto done;

                default:
                    goto done;
            }
        }
    }

done:
    tiremo_lis2de12tr_i2c_set_op_stop(ptI2c);
    SET_I2C_SR_CLEAR(ptI2c);

    if (bBusLocked)
    {
        tiremo_lis2de12tr_i2c_bus_end(eId, ptI2c);
    }

    if (eResult == HAL_ERR_OK)
    {
        s_i2cFailStreak = 0U;
    }
    else
    {
        tiremo_lis2de12tr_i2c_on_error(eId);
    }

    return eResult;
}
