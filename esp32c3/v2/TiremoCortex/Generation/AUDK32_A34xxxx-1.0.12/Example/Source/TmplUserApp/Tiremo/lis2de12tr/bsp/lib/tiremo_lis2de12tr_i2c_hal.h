/**
 *******************************************************************************
 * @file        tiremo_lis2de12tr_i2c_hal.h
 * @brief       Tiremo LIS2DE12TR I2C hardware abstraction layer API
 *
 * @details     Provides repeated-START I2C write-read transactions required by
 *              the LIS2DE12 register interface.
 *******************************************************************************
 */

#ifndef TIREMO_LIS2DE12TR_I2C_HAL_H_
#define TIREMO_LIS2DE12TR_I2C_HAL_H_

#include <stdint.h>
#include "hal_i2c.h"

/**
 * @brief       Combined I2C write-read with repeated START (polling).
 * @param[in]   eId          I2C peripheral instance.
 * @param[in]   un8SlaveAddr 7-bit slave address.
 * @param[in]   pun8Out      Write buffer.
 * @param[in]   un32WrLen    Number of bytes to write.
 * @param[out]  pun8In       Read buffer.
 * @param[in]   un32RdLen    Number of bytes to read.
 * @return      HAL_ERR_OK on success, error code otherwise.
 * @note        START -> addr+W -> write -> repeated START -> addr+R -> read -> STOP
 */
HAL_ERR_e tiremo_lis2de12tr_i2c_write_read(I2C_ID_e eId, uint8_t un8SlaveAddr,
                                           uint8_t* pun8Out, uint32_t un32WrLen,
                                           uint8_t* pun8In, uint32_t un32RdLen);

/**
 * @brief       Release a potentially stuck I2C bus after MCU reset.
 * @param[in]   eId  I2C peripheral instance.
 * @return      None
 */
void tiremo_lis2de12tr_i2c_prepare_bus(I2C_ID_e eId);

/**
 * @brief       Full I2C peripheral re-init after repeated transfer errors.
 * @param[in]   eId  I2C peripheral instance.
 * @return      None
 */
void tiremo_lis2de12tr_i2c_full_recover(I2C_ID_e eId);

#endif /* TIREMO_LIS2DE12TR_I2C_HAL_H_ */
