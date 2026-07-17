/**
 *******************************************************************************
 * @file        tiremo_lis2de12tr_bsp.h
 * @brief       Tiremo LIS2DE12TR board support package API
 *
 * @details     Provides hardware-facing routines for LIS2DE12TR initialization,
 *              acceleration measurement, and INT1/INT2 interrupt handling.
 *******************************************************************************
 */

#ifndef TIREMO_LIS2DE12TR_BSP_H_
#define TIREMO_LIS2DE12TR_BSP_H_

#include <stdbool.h>
#include <stdint.h>

/**
 * @brief LIS2DE12TR interrupt line identifiers.
 */
typedef enum
{
    TIREMO_LIS2DE12TR_INTR_INT1 = 0, /**< PA4 / INT1 (data-ready). */
    TIREMO_LIS2DE12TR_INTR_INT2,     /**< PA5 / INT2 (motion). */
    TIREMO_LIS2DE12TR_INTR_COUNT
} TiremoLis2de12trIntrId_e;

/**
 * @brief       Initialize LIS2DE12TR BSP and sensor.
 * @param[in]   None
 * @return      true on success, false on failure.
 * @note        I2C port/pin configuration is done in MCU Brew32 (user_i2c.c).
 */
bool TIREMO_LIS2DE12TR_BSP_Init(void);

/**
 * @brief       Configure sensor interrupt routing and enable PA4/PA5 GPIO IRQs.
 * @param[in]   None
 * @return      true on success, false on failure.
 * @note        Call after TIREMO_LIS2DE12TR_BSP_Init().
 */
bool TIREMO_LIS2DE12TR_BSP_Intr_Init(void);

/**
 * @brief       Read acceleration on X/Y/Z axes in milli-g.
 * @param[out]  accel_x_mg  X-axis acceleration in mg.
 * @param[out]  accel_y_mg  Y-axis acceleration in mg.
 * @param[out]  accel_z_mg  Z-axis acceleration in mg.
 * @return      true on success, false on failure.
 */
bool TIREMO_LIS2DE12TR_BSP_ReadAccel(int16_t* accel_x_mg, int16_t* accel_y_mg,
                                     int16_t* accel_z_mg);

/**
 * @brief       Read one acceleration sample without waiting for DRDY.
 * @param[out]  accel_x_mg  X-axis acceleration in mg.
 * @param[out]  accel_y_mg  Y-axis acceleration in mg.
 * @param[out]  accel_z_mg  Z-axis acceleration in mg.
 * @return      true on success, false on failure.
 */
bool TIREMO_LIS2DE12TR_BSP_ReadAccelOnce(int16_t* accel_x_mg, int16_t* accel_y_mg,
                                         int16_t* accel_z_mg);

/**
 * @brief       Poll sensor data-ready and set INT1 pending (main loop only).
 * @param[in]   None
 * @return      None
 */
void TIREMO_LIS2DE12TR_BSP_Intr_Service(void);

/**
 * @brief       Recover I2C bus and sensor after repeated communication errors.
 * @param[in]   None
 * @return      true on success, false on failure.
 */
bool TIREMO_LIS2DE12TR_BSP_RecoverRuntime(void);

/**
 * @brief       Check whether an interrupt event is pending.
 * @param[in]   intrId  Interrupt line (INT1 or INT2).
 * @return      true if pending, false otherwise.
 */
bool TIREMO_LIS2DE12TR_BSP_Intr_IsPending(TiremoLis2de12trIntrId_e intrId);

/**
 * @brief       Clear a pending interrupt event flag.
 * @param[in]   intrId  Interrupt line (INT1 or INT2).
 * @return      None
 */
void TIREMO_LIS2DE12TR_BSP_Intr_Clear(TiremoLis2de12trIntrId_e intrId);

/**
 * @brief       Deinitialize LIS2DE12TR BSP resources.
 * @param[in]   None
 * @return      None
 */
void TIREMO_LIS2DE12TR_BSP_Deinit(void);

#endif /* TIREMO_LIS2DE12TR_BSP_H_ */
