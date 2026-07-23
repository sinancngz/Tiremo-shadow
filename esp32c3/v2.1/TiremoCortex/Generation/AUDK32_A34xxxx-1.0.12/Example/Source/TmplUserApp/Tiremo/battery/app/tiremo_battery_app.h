/**
 *******************************************************************************
 * @file        tiremo_battery_app.h
 * @brief       Tiremo battery application layer
 *
 * @details     AVDD measurement, formatting, and optional UART output hook.
 *
 * @note        BSP has no APP dependency. app.c includes bsp.h only.
 *
 ******************************************************************************/

#ifndef TIREMO_BATTERY_APP_H_
#define TIREMO_BATTERY_APP_H_

#include <stdbool.h>
#include <stdint.h>

/* ========================================================================== */
/* Application configuration — edit for your product                         */
/* ========================================================================== */

/** Internal VCORE reference voltage (mV). Bandgap ~1.0 V on AUDK32. */
#define TIREMO_BAT_VCORE_REF_MV         (1000U)

/** ADC full scale for AVDD formula (12-bit: 4096). */
#define TIREMO_BAT_ADC_FULL_SCALE       (4096U)

/**
 * Optional one-point calibration (0 = use formula only).
 * If you know true AVDD (mV) and see vcoreRaw R at runtime:
 *   CAL = (trueAvddMv * R) / TIREMO_BAT_ADC_FULL_SCALE
 * Example: 3500 mV supply, raw 1170 → CAL ≈ 1000.
 */
#define TIREMO_BAT_VCORE_CALIB_MV       (0U)

/** Main loop delay between measurements (ms). */
#define TIREMO_BAT_APP_POLL_MS          (1000U)

/** @brief One battery / supply voltage reading. */
typedef struct
{
    uint16_t vcoreRaw;
    uint32_t avddMv;
    bool valid;
} TiremoBatReading_t;

/**
 * @brief   Initialize APP + BSP (after PRV_ADC_Init).
 * @return  true on success.
 */
bool TIREMO_BAT_App_Init(void);

/**
 * @brief       Measure AVDD via VCORE channel.
 * @param[out]  pReading  Filled on success (.valid == true).
 * @return      true if measurement succeeded.
 */
bool TIREMO_BAT_App_Read(TiremoBatReading_t *pReading);

/**
 * @brief   Read AVDD, format line, call OnDebugOutput() (override in prv_user_code.c).
 * @return  true if measurement succeeded.
 */
bool TIREMO_BAT_App_Update(void);

/** @brief Last successful AVDD in millivolts (0 if never read). */
uint32_t TIREMO_BAT_App_GetAvddMv(void);

/** @brief Last VCORE raw ADC value. */
uint16_t TIREMO_BAT_App_GetVcoreRaw(void);

/**
 * @brief       Debug output hook — one formatted line per successful Update().
 * @details     Weak default; override in prv_user_code.c for UART output.
 */
void TIREMO_BAT_App_OnDebugOutput(const char *line);

#endif /* TIREMO_BATTERY_APP_H_ */
