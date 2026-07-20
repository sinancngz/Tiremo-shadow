/**
 ******************************************************************************
 * @file           : slm320.h
 * @brief          : MeiG SLM320 4G LTE modem driver (ABOV A34G43x)
 ******************************************************************************
 * Network/SSL : QICSGP, QIACT, QSSLCFG, QFUPL (SLM3XX SSL manual)
 * MQTT plain   : AT+MQTTCONN, AT+MQTTPUB (connectmqtt.txt, port 1883)
 * MQTT TLS     : QMTCFG + QMTOPEN + QMTCONN + QMTPUBEX (ssl.txt + Quectel QMT)
 *
 * Hardware (board pins in board_config.h):
 *   PA7  = PWRKEY  (LOW >= 1s to turn on)
 *   PC4  = PWR     (CLEAR = enable module power supply)
 *   UART = SLM320_UART_ID  (default UART_ID_1, 115200 8N1)
 ******************************************************************************
 */

#ifndef __SLM320_H
#define __SLM320_H

#ifdef __cplusplus
extern "C" {
#endif

#include "hal_uart.h"
#include "hal_pcu.h"
#include "../../config/mqtt_device_config.h"
#include <stdint.h>

typedef enum
{
    SLM320_STATE_IDLE = 0,
    SLM320_STATE_POWER_ENABLE,
    SLM320_STATE_CHECK_READY,
    SLM320_STATE_CHECK_AT,
    SLM320_STATE_CHECK_CPIN,
    SLM320_STATE_CHECK_CREG,
    SLM320_STATE_SET_APN,
    SLM320_STATE_ENABLE_QIACT,
    SLM320_STATE_SSL_CFG,
    SLM320_STATE_MQTT_OPEN,
    SLM320_STATE_MQTT_CONNECT,
    SLM320_STATE_GET_IMEI,
    SLM320_STATE_MQTT_PUBLISH,
    SLM320_STATE_MQTT_DISCONNECT,
    SLM320_STATE_POWER_OFF,
    SLM320_STATE_RESET,
    SLM320_STATE_DONE,
    SLM320_STATE_ERROR
} SLM320_State_t;

typedef enum
{
    SLM320_OK = 0,
    SLM320_ERROR,
    SLM320_TIMEOUT
} SLM320_Status_t;

#define SLM320_UART_ID          UART_ID_1
#define SLM320_RX_BUF_SIZE      4096U
#define SLM320_MQTT_MSG_MAX     560U
#define SLM320_MQTT_CLIENT_ID     0U
#define SLM320_SSL_CTX_ID       2U
/* TLS MQTT: QMTCFG links MQTT client 0 to SSL context 2. */

#define SLM320_PWRKEY_PORT      PCU_ID_A
#define SLM320_PWRKEY_PIN       PCU_PIN_ID_7

#define SLM320_PWR_PORT         PCU_ID_C
#define SLM320_PWR_PIN          PCU_PIN_ID_4

extern SLM320_State_t     slm320_state;
extern uint8_t            slm320_rx_buf[SLM320_RX_BUF_SIZE];
extern volatile uint16_t  slm320_rx_len;
extern char               slm320_imei[16];

void            SLM320_TickIncrement(void);
void            SLM320_Init(uint32_t (*pfnGetTick)(void));
void            SLM320_SendCmd(const char *cmd);
uint8_t         SLM320_CheckResponse(const char *expected, uint32_t timeout_ms);
void            SLM320_ResetRxBuffer(void);
void            SLM320_PowerOn(void);
void            SLM320_PowerOff(void);
void            SLM320_Reset(void);
SLM320_Status_t SLM320_RunStateMachine(void);
SLM320_Status_t SLM320_PublishSensorData(const char *topic, const char *data);
SLM320_Status_t SLM320_GetIMEI(void);
void            SLM320_UART_RxCallback(void);

#if MQTT_USE_TLS_CERTS
/** Power on modem and wait for AT (for cert upload before full connect). */
uint8_t         SLM320_BootstrapAt(void);
/** Upload CA/client/key PEM files to modem UFS via QFUPL. */
uint8_t         SLM320_CertsUploadAll(void);
/** Verify cacert.pem, client.pem, user_key.pem exist on modem. */
uint8_t         SLM320_CertsVerifyAll(void);
#endif

#ifdef __cplusplus
}
#endif

#endif /* __SLM320_H */
