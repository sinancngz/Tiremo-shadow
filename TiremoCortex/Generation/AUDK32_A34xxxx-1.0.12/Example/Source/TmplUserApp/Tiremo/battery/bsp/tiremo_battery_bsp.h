/**
 *******************************************************************************
 * @file        tiremo_battery_bsp.h
 * @brief       Tiremo battery BSP — ADC hardware read layer
 *
 * @details     Responsibilities:
 *              - Configure sequence, start conversion, poll, read raw value (HAL_ADC_*)
 *
 * @note        No APP dependency. ADC init is in user_adc.c.
 *
 ******************************************************************************/

#ifndef TIREMO_BATTERY_BSP_H_
#define TIREMO_BATTERY_BSP_H_

#include <stdbool.h>
#include <stdint.h>

#include "tiremo_battery_board.h"

/**
 * @brief   Wait for VCORE reference stabilization after PRV_ADC_Init().
 */
void TIREMO_BAT_BSP_Init(void);

/**
 * @brief       Read one ADC channel (single conversion, POLL).
 * @param[in]   channel  ADC channel number (0–N or internal e.g. VCORE).
 * @param[out]  pRaw     12-bit result.
 * @return      true on success.
 */
bool TIREMO_BAT_BSP_ReadChannel(uint8_t channel, uint16_t *pRaw);

/**
 * @brief       Read internal VCORE reference channel.
 * @param[out]  pRaw  12-bit VCORE ADC value.
 * @return      true on success.
 */
bool TIREMO_BAT_BSP_ReadVcore(uint16_t *pRaw);

#endif /* TIREMO_BATTERY_BSP_H_ */
