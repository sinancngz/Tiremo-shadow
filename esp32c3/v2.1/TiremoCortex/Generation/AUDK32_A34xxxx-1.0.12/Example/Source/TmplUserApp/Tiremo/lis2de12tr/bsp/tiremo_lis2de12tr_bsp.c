#include "abov_config.h"
#include "abov_module_config.h"
#include <stddef.h>
#include "hal_pcu.h"
#include "hal_i2c.h"
#include "tiremo_lis2de12tr_bsp.h"
#include "tiremo_lis2de12tr_board.h"
#include "lib/tiremo_lis2de12tr_driver.h"
#include "lib/tiremo_lis2de12tr_i2c_hal.h"
#include "../../common/tiremo_systick.h"

#define TIREMO_LIS2DE12TR_CTRL1_DESIRED   (0x5FU)
#define TIREMO_LIS2DE12TR_OVERSAMPLE_CNT  (4U)
#define TIREMO_LIS2DE12TR_I2C_ID          I2C_ID_2
#define TIREMO_LIS2DE12TR_BUS_SETTLE_MS   (100U)
#define TIREMO_LIS2DE12TR_BOOT_DELAY_MS   (50U)
#define TIREMO_LIS2DE12TR_WHOAMI_RETRIES  (10U)
#define TIREMO_LIS2DE12TR_HEALTH_FAIL_MAX (3U)

static bool s_tiremoLis2de12Ready = false;
static bool s_tiremoLis2de12IntrReady = false;
static bool s_portIrqRegistered = false;

static volatile uint8_t s_intrPending[TIREMO_LIS2DE12TR_INTR_COUNT];
static uint8_t s_healthFailStreak = 0U;

bool TIREMO_LIS2DE12TR_BSP_RecoverRuntime(void);

static bool TIREMO_LIS2DE12TR_BSP_ConfigureSensorInterrupts(void);

static bool TIREMO_LIS2DE12TR_BSP_IsValidIntrId(TiremoLis2de12trIntrId_e intrId) {
	return ((uint32_t) intrId < TIREMO_LIS2DE12TR_INTR_COUNT);
}

static bool TIREMO_LIS2DE12TR_BSP_VerifyCtrl1(void) {
	uint8_t ctrl1_desired = TIREMO_LIS2DE12TR_CTRL1_DESIRED;
	uint8_t ctrl1_readback = 0U;
	uint8_t attempt;

	for (attempt = 0U; attempt < 5U; attempt++) {
		if (lis2de12_write_reg(LIS2DE12_CTRL_REG1, &ctrl1_desired, 1) != 0) {
			TIREMO_SysTick_DelayMs(10U);
			continue;
		}

		TIREMO_SysTick_DelayMs(2U);
		if ((lis2de12_read_reg(LIS2DE12_CTRL_REG1, &ctrl1_readback, 1) == 0)
				&& (ctrl1_readback == ctrl1_desired)) {
			return true;
		}

		TIREMO_SysTick_DelayMs(10U);
	}

	return false;
}

static bool TIREMO_LIS2DE12TR_BSP_ProbeWhoAmI(uint8_t *whoami) {
	uint8_t retry;
	int32_t error;

	for (retry = 0U; retry < TIREMO_LIS2DE12TR_WHOAMI_RETRIES; retry++) {
		if (retry > 0U) {
			tiremo_lis2de12tr_i2c_prepare_bus(TIREMO_LIS2DE12TR_I2C_ID);
			(void) lis2de12_boot_set(PROPERTY_ENABLE);
			TIREMO_SysTick_DelayMs(TIREMO_LIS2DE12TR_BOOT_DELAY_MS);
		}

		error = lis2de12_device_id_get(whoami);
		if ((error == 0) && (*whoami == LIS2DE12_ID)) {
			return true;
		}

		TIREMO_SysTick_DelayMs(10U);
	}

	return false;
}

static void TIREMO_LIS2DE12TR_BSP_MarkI2cOk(void) {
	s_healthFailStreak = 0U;
}

static void TIREMO_LIS2DE12TR_BSP_MarkI2cFail(void) {
	if (s_healthFailStreak < 0xFFU) {
		s_healthFailStreak++;
	}

	if (s_healthFailStreak >= TIREMO_LIS2DE12TR_HEALTH_FAIL_MAX) {
		(void) TIREMO_LIS2DE12TR_BSP_RecoverRuntime();
	}
}

bool TIREMO_LIS2DE12TR_BSP_RecoverRuntime(void) {
	uint8_t whoami = 0U;

	s_healthFailStreak = 0U;
	s_intrPending[TIREMO_LIS2DE12TR_INTR_INT1] = 0U;
	s_intrPending[TIREMO_LIS2DE12TR_INTR_INT2] = 0U;

	tiremo_lis2de12tr_i2c_full_recover(TIREMO_LIS2DE12TR_I2C_ID);
	TIREMO_SysTick_DelayMs(10U);
	(void) lis2de12_boot_set(PROPERTY_ENABLE);
	TIREMO_SysTick_DelayMs(TIREMO_LIS2DE12TR_BOOT_DELAY_MS);

	if (!TIREMO_LIS2DE12TR_BSP_ProbeWhoAmI(&whoami)) {
		return false;
	}

	if (!TIREMO_LIS2DE12TR_BSP_VerifyCtrl1()) {
		return false;
	}

	if (s_tiremoLis2de12IntrReady) {
		(void) TIREMO_LIS2DE12TR_BSP_ConfigureSensorInterrupts();
		(void) TIREMO_LIS2DE12TR_BSP_VerifyCtrl1();
	}

	return true;
}

static bool TIREMO_LIS2DE12TR_BSP_ConfigureSensorInterrupts(void) {
	lis2de12_int1_cfg_t int1_cfg = { 0 };
	lis2de12_ctrl_reg3_t ctrl_reg3 = { 0 };
	lis2de12_ctrl_reg6_t ctrl_reg6 = { 0 };

	if (lis2de12_high_pass_mode_set(LIS2DE12_AUTORST_ON_INT) != 0) {
		return false;
	}

	if (lis2de12_high_pass_bandwidth_set(LIS2DE12_LIGHT) != 0) {
		return false;
	}

	if (lis2de12_high_pass_on_outputs_set(PROPERTY_DISABLE) != 0) {
		return false;
	}

	if (lis2de12_high_pass_int_conf_set(LIS2DE12_ON_INT1_GEN) != 0) {
		return false;
	}

	if (lis2de12_int1_gen_threshold_set(10) != 0) {
		return false;
	}

	if (lis2de12_int1_gen_duration_set(1) != 0) {
		return false;
	}

	int1_cfg.xlie = 1U;
	int1_cfg.ylie = 1U;
	int1_cfg.zlie = 1U;
	if (lis2de12_int1_gen_conf_set(&int1_cfg) != 0) {
		return false;
	}

	ctrl_reg3.i1_zyxda = 1U;
	if (lis2de12_pin_int1_config_set(&ctrl_reg3) != 0) {
		return false;
	}

	ctrl_reg6.i2_ia1 = 1U;
	if (lis2de12_pin_int2_config_set(&ctrl_reg6) != 0) {
		return false;
	}

	return true;
}

static void TIREMO_LIS2DE12TR_BSP_Intr_ClearSensorStatus(
		TiremoLis2de12trIntrId_e intrId) {
	lis2de12_int1_src_t int1_src = { 0 };

	if (intrId == TIREMO_LIS2DE12TR_INTR_INT2) {
		/* Motion is INT1 generator routed to INT2 pin (i2_ia1). */
		(void) lis2de12_int1_gen_source_get(&int1_src);
	}
}

static void TIREMO_LIS2DE12TR_BSP_Intr_PortHandler(uint32_t un32Event,
		void *pContext) {
	(void) pContext;

	if ((un32Event & TIREMO_LIS2DE12TR_INT1_INTR_MASK) != 0U) {
		s_intrPending[TIREMO_LIS2DE12TR_INTR_INT1] = 1U;
	}

	if ((un32Event & TIREMO_LIS2DE12TR_INT2_INTR_MASK) != 0U) {
		s_intrPending[TIREMO_LIS2DE12TR_INTR_INT2] = 1U;
	}
}

bool TIREMO_LIS2DE12TR_BSP_Init(void) {
	uint8_t whoami = 0U;
	uint8_t dummy[6];
	uint32_t index;

	if (s_tiremoLis2de12Ready) {
		return true;
	}

	for (index = 0U; index < TIREMO_LIS2DE12TR_INTR_COUNT; index++) {
		s_intrPending[index] = 0U;
	}

	/* MCU soft-reset keeps sensor powered; recover bus and reboot sensor. */
	TIREMO_SysTick_DelayMs(TIREMO_LIS2DE12TR_BUS_SETTLE_MS);
	tiremo_lis2de12tr_i2c_prepare_bus(TIREMO_LIS2DE12TR_I2C_ID);
	(void) lis2de12_boot_set(PROPERTY_ENABLE);
	TIREMO_SysTick_DelayMs(TIREMO_LIS2DE12TR_BOOT_DELAY_MS);

	if (!TIREMO_LIS2DE12TR_BSP_ProbeWhoAmI(&whoami)) {
		return false;
	}

	if (lis2de12_data_rate_set(LIS2DE12_POWER_DOWN) != 0) {
		return false;
	}
	TIREMO_SysTick_DelayMs(5U);

	if (lis2de12_block_data_update_set(PROPERTY_DISABLE) != 0) {
		return false;
	}

	if (lis2de12_full_scale_set(LIS2DE12_2g) != 0) {
		return false;
	}

	if (lis2de12_temperature_meas_set(LIS2DE12_TEMP_ENABLE) != 0) {
		return false;
	}

	if (!TIREMO_LIS2DE12TR_BSP_VerifyCtrl1()) {
		return false;
	}

	TIREMO_SysTick_DelayMs(20U);
	(void) lis2de12_read_reg(0x29U, dummy, 6U);

	s_tiremoLis2de12Ready = true;
	return true;
}

bool TIREMO_LIS2DE12TR_BSP_Intr_Init(void) {
	PCU_IRQ_CFG_t irqCfg;

	if (!s_tiremoLis2de12Ready) {
		return false;
	}

	if (s_tiremoLis2de12IntrReady) {
		return true;
	}

	if (!TIREMO_LIS2DE12TR_BSP_ConfigureSensorInterrupts()) {
		return false;
	}

	if (!TIREMO_LIS2DE12TR_BSP_VerifyCtrl1()) {
		return false;
	}

	(void) HAL_PCU_SetIntrPort(TIREMO_LIS2DE12TR_INT1_PORT,
	TIREMO_LIS2DE12TR_INT1_PIN, PCU_INTR_MODE_EDGE,
			PCU_INTR_TRG_BOTH_LEVEL_EDGE, 0U);

	(void) HAL_PCU_SetIntrPort(TIREMO_LIS2DE12TR_INT2_PORT,
	TIREMO_LIS2DE12TR_INT2_PIN, PCU_INTR_MODE_EDGE,
			PCU_INTR_TRG_BOTH_LEVEL_EDGE, 0U);

	if (!s_portIrqRegistered) {
		irqCfg.eId = TIREMO_LIS2DE12TR_INT1_PORT;
		irqCfg.eOps = PCU_OPS_INTR;
		irqCfg.pfnHandler = TIREMO_LIS2DE12TR_BSP_Intr_PortHandler;
		irqCfg.pContext = NULL;
		irqCfg.un32IRQPrio = TIREMO_LIS2DE12TR_IRQ_PRIO;
		irqCfg.un8IntNum = 0U;
		(void) HAL_PCU_SetIRQ(&irqCfg);
		s_portIrqRegistered = true;
	}

	s_tiremoLis2de12IntrReady = true;
	return true;
}

bool TIREMO_LIS2DE12TR_BSP_ReadAccelOnce(int16_t *accel_x_mg,
		int16_t *accel_y_mg, int16_t *accel_z_mg) {
	uint8_t raw[6];
	int32_t ret;
	int16_t raw_x;
	int16_t raw_y;
	int16_t raw_z;

	if ((!s_tiremoLis2de12Ready) || (accel_x_mg == NULL) || (accel_y_mg == NULL)
			|| (accel_z_mg == NULL)) {
		return false;
	}

	ret = lis2de12_read_reg(0x29U, raw, 6U);
	if (ret != 0) {
		TIREMO_LIS2DE12TR_BSP_MarkI2cFail();
		return false;
	}

	TIREMO_LIS2DE12TR_BSP_MarkI2cOk();

	raw_x = (int16_t) (((uint16_t) raw[0] << 8) | raw[1]);
	raw_y = (int16_t) (((uint16_t) raw[2] << 8) | raw[3]);
	raw_z = (int16_t) (((uint16_t) raw[4] << 8) | raw[5]);

	*accel_x_mg = (int16_t) (raw_x * 125 / 2048);
	*accel_y_mg = (int16_t) (raw_y * 125 / 2048);
	*accel_z_mg = (int16_t) (raw_z * 125 / 2048);

	return true;
}

bool TIREMO_LIS2DE12TR_BSP_ReadAccel(int16_t *accel_x_mg, int16_t *accel_y_mg,
		int16_t *accel_z_mg) {
	uint8_t raw[6];
	uint8_t status_reg = 0U;
	uint8_t n;
	int32_t sum_x = 0;
	int32_t sum_y = 0;
	int32_t sum_z = 0;
	uint16_t timeout;
	int32_t ret;

	if ((!s_tiremoLis2de12Ready) || (accel_x_mg == NULL) || (accel_y_mg == NULL)
			|| (accel_z_mg == NULL)) {
		return false;
	}

	for (n = 0U; n < TIREMO_LIS2DE12TR_OVERSAMPLE_CNT; n++) {
		timeout = 200U;
		do {
			ret = lis2de12_read_reg(LIS2DE12_STATUS_REG, &status_reg, 1U);
			if (ret != 0) {
				return false;
			}
			if ((status_reg & 0x08U) != 0U) {
				break;
			}
			TIREMO_SysTick_DelayMs(1U);
		} while (--timeout > 0U);

		if ((status_reg & 0x08U) == 0U) {
			return false;
		}

		ret = lis2de12_read_reg(0x29U, raw, 6U);
		if (ret != 0) {
			return false;
		}

		sum_x += (int16_t) (((uint16_t) raw[0] << 8) | raw[1]);
		sum_y += (int16_t) (((uint16_t) raw[2] << 8) | raw[3]);
		sum_z += (int16_t) (((uint16_t) raw[4] << 8) | raw[5]);
	}

	*accel_x_mg = (int16_t) (sum_x * 125
			/ (TIREMO_LIS2DE12TR_OVERSAMPLE_CNT * 2048));
	*accel_y_mg = (int16_t) (sum_y * 125
			/ (TIREMO_LIS2DE12TR_OVERSAMPLE_CNT * 2048));
	*accel_z_mg = (int16_t) (sum_z * 125
			/ (TIREMO_LIS2DE12TR_OVERSAMPLE_CNT * 2048));

	return true;
}

bool TIREMO_LIS2DE12TR_BSP_Intr_IsPending(TiremoLis2de12trIntrId_e intrId) {
	if (TIREMO_LIS2DE12TR_BSP_IsValidIntrId(intrId)) {
		return (s_intrPending[intrId] != 0U);
	}

	return false;
}

void TIREMO_LIS2DE12TR_BSP_Intr_Service(void) {
	uint8_t drdy = 0U;
	int32_t ret;

	if (!s_tiremoLis2de12IntrReady) {
		return;
	}

	if (s_intrPending[TIREMO_LIS2DE12TR_INTR_INT1] == 0U) {
		ret = lis2de12_xl_data_ready_get(&drdy);
		if ((ret == 0) && (drdy != 0U)) {
			s_intrPending[TIREMO_LIS2DE12TR_INTR_INT1] = 1U;
			TIREMO_LIS2DE12TR_BSP_MarkI2cOk();
		} else if (ret != 0) {
			TIREMO_LIS2DE12TR_BSP_MarkI2cFail();
		}
	}
}

void TIREMO_LIS2DE12TR_BSP_Intr_Clear(TiremoLis2de12trIntrId_e intrId) {
	if (TIREMO_LIS2DE12TR_BSP_IsValidIntrId(intrId)) {
		s_intrPending[intrId] = 0U;
		TIREMO_LIS2DE12TR_BSP_Intr_ClearSensorStatus(intrId);
	}
}

void TIREMO_LIS2DE12TR_BSP_Deinit(void) {
	s_tiremoLis2de12Ready = false;
	s_tiremoLis2de12IntrReady = false;
	s_portIrqRegistered = false;
	s_healthFailStreak = 0U;
}
