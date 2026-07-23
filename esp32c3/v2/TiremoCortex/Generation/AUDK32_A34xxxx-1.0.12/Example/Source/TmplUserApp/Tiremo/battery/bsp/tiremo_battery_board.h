/**
 *******************************************************************************
 * @file        tiremo_battery_board.h
 * @brief       Board configuration — Tiremo battery ADC on AUDK32
 *
 * @details     BSP hardware constants only. APP settings: tiremo_battery_app.h.
 *
 * @note        ADC peripheral init: user_adc.c (ADC_ID_0, SINGLE, POLL).
 *              Internal VCORE on channel 23.
 *
 ******************************************************************************/

#ifndef TIREMO_BATTERY_BOARD_H_
#define TIREMO_BATTERY_BOARD_H_

#include "hal_adc.h"

/* ========================================================================== */
/* Hardware mapping (must match adc_configuration.yaml / user_adc.c)       */
/* ========================================================================== */

#define TIREMO_BAT_ADC_ID               ADC_ID_1

/** Internal 1.0 V VCORE reference channel (AVDD calculation). */
#define TIREMO_BAT_VCORE_CHANNEL        (23U)

/** Sequence 0 sampling time (INDEPENDENT trigger path). */
#define TIREMO_BAT_ADC_SAMPLING_TIME    (10U)

/** Post-Start busy-wait loops. */
#define TIREMO_BAT_ADC_CONV_DELAY_LOOPS   (10000U)

/** Busy-wait loops after BSP Init for VCORE stabilization. */
#define TIREMO_BAT_VCORE_STABILIZE_LOOPS (500000U)

#endif /* TIREMO_BATTERY_BOARD_H_ */
