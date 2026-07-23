/**
 *******************************************************************************
 * @file        tiremo_lis2de12tr_app.h
 * @brief       Tiremo LIS2DE12TR application layer API
 *
 * @details     Thin application layer that delegates to LIS2DE12TR BSP routines.
 *******************************************************************************
 */

#ifndef TIREMO_LIS2DE12TR_APP_H_
#define TIREMO_LIS2DE12TR_APP_H_

#include <stdbool.h>
#include <stdint.h>

#include "../bsp/tiremo_lis2de12tr_bsp.h"

/**
 * @brief       Initialize LIS2DE12TR application layer.
 * @param[in]   None
 * @return      true on success, false on failure.
 */
bool TIREMO_LIS2DE12TR_App_Init(void);

/**
 * @brief       Initialize LIS2DE12TR interrupt handling.
 * @param[in]   None
 * @return      true on success, false on failure.
 */
bool TIREMO_LIS2DE12TR_App_Intr_Init(void);

/**
 * @brief       Read acceleration through BSP.
 * @param[out]  accel_x_mg  X-axis acceleration in mg.
 * @param[out]  accel_y_mg  Y-axis acceleration in mg.
 * @param[out]  accel_z_mg  Z-axis acceleration in mg.
 * @return      true on success, false on failure.
 */
bool TIREMO_LIS2DE12TR_App_ReadAccel(int16_t* accel_x_mg, int16_t* accel_y_mg,
                                     int16_t* accel_z_mg);

bool TIREMO_LIS2DE12TR_App_ReadAccelOnce(int16_t* accel_x_mg, int16_t* accel_y_mg,
                                         int16_t* accel_z_mg);

void TIREMO_LIS2DE12TR_App_Intr_Service(void);

bool TIREMO_LIS2DE12TR_App_RecoverRuntime(void);

/**
 * @brief       Check whether an interrupt event is pending.
 * @param[in]   intrId  Interrupt line (INT1 or INT2).
 * @return      true if pending, false otherwise.
 */
bool TIREMO_LIS2DE12TR_App_Intr_IsPending(TiremoLis2de12trIntrId_e intrId);

/**
 * @brief       Clear a pending interrupt event flag.
 * @param[in]   intrId  Interrupt line (INT1 or INT2).
 * @return      None
 */
void TIREMO_LIS2DE12TR_App_Intr_Clear(TiremoLis2de12trIntrId_e intrId);

#endif /* TIREMO_LIS2DE12TR_APP_H_ */
