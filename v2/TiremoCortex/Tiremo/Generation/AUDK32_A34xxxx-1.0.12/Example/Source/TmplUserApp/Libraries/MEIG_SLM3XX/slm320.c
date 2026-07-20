/**
 ******************************************************************************
 * @file           : slm320.c
 * @brief          : MeiG SLM320 4G LTE modem driver (ABOV A34G43x)
 ******************************************************************************
 */

#include "slm320.h"
#include "../../config/project_config.h"
#if MQTT_USE_TLS_CERTS
#include "../cert_Lib/mqtt_certs.h"
#endif
#include "system_a34xxxx.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static uint32_t (*s_pfnGetTick)(void) = NULL;
static volatile uint32_t s_tick_ms = 0U;

SLM320_State_t     slm320_state = SLM320_STATE_IDLE;
uint8_t            slm320_rx_buf[SLM320_RX_BUF_SIZE];
volatile uint16_t  slm320_rx_len = 0;
char               slm320_imei[16] = {0};

static uint8_t s_rx_byte;

static void *slm320_memmem(const void *haystack, size_t haystacklen,
                           const void *needle, size_t needlelen);
static uint32_t slm320_get_tick_ms(void);
static void slm320_log(const char *msg);
static uint8_t slm320_upload_pem(const char *filename, const char *pem_data, size_t pem_len);
static uint8_t slm320_verify_cert_file(const char *filename);
static uint8_t slm320_ssl_send_cmd(const char *cmd);
static uint8_t slm320_qmt_send_cfg(const char *cmd);
static void slm320_mqtt_disconnect_prep(void);
static uint8_t slm320_configure_dns(void);
static uint8_t slm320_dns_resolve(const char *host);
static void slm320_log_rx_error(const char *stage);

extern void SYSTICK_Wait(uint32_t un32TimeMS);

void SLM320_TickIncrement(void)
{
    s_tick_ms++;
}

static uint32_t slm320_get_tick_ms(void)
{
    if (s_pfnGetTick != NULL)
        return s_pfnGetTick();
    return s_tick_ms;
}

static void slm320_log(const char *msg)
{
    HAL_UART_Transmit(UART_ID_0, (uint8_t *)msg, (uint32_t)strlen(msg), true);
}

void SLM320_Init(uint32_t (*pfnGetTick)(void))
{
    s_pfnGetTick = pfnGetTick;
    slm320_state = SLM320_STATE_IDLE;
    slm320_rx_len = 0;
    memset(slm320_rx_buf, 0, SLM320_RX_BUF_SIZE);

    HAL_PCU_SetInOutMode(SLM320_PWRKEY_PORT, SLM320_PWRKEY_PIN,
                         PCU_INOUT_OUTPUT_PUSH_PULL);
    HAL_PCU_SetAltMode(SLM320_PWRKEY_PORT, SLM320_PWRKEY_PIN, PCU_ALT_0);
    HAL_PCU_SetOutputBit(SLM320_PWRKEY_PORT, SLM320_PWRKEY_PIN, PCU_OUTPUT_BIT_SET);

    HAL_PCU_SetInOutMode(SLM320_PWR_PORT, SLM320_PWR_PIN,
                         PCU_INOUT_OUTPUT_PUSH_PULL);
    HAL_PCU_SetAltMode(SLM320_PWR_PORT, SLM320_PWR_PIN, PCU_ALT_0);
    HAL_PCU_SetOutputBit(SLM320_PWR_PORT, SLM320_PWR_PIN, PCU_OUTPUT_BIT_SET);

    HAL_UART_Receive(SLM320_UART_ID, &s_rx_byte, 1, false);
}

void SLM320_SendCmd(const char *cmd)
{
    HAL_UART_Transmit(SLM320_UART_ID, (uint8_t *)cmd, (uint32_t)strlen(cmd), true);
}

uint8_t SLM320_CheckResponse(const char *expected, uint32_t timeout_ms)
{
    uint32_t start = slm320_get_tick_ms();
    size_t exp_len = strlen(expected);

    while ((slm320_get_tick_ms() - start) < timeout_ms)
    {
        if (slm320_rx_len == 0)
            continue;
        if (slm320_memmem(slm320_rx_buf, slm320_rx_len, expected, exp_len) != NULL)
            return 1U;
    }
    return 0U;
}

void SLM320_ResetRxBuffer(void)
{
    HAL_UART_Abort(SLM320_UART_ID);
    memset(slm320_rx_buf, 0, SLM320_RX_BUF_SIZE);
    slm320_rx_len = 0;
    HAL_UART_Receive(SLM320_UART_ID, &s_rx_byte, 1, false);
}

void SLM320_PowerOn(void)
{
    HAL_PCU_SetOutputBit(SLM320_PWR_PORT, SLM320_PWR_PIN, PCU_OUTPUT_BIT_CLEAR);
    SYSTICK_Wait(500);

    SLM320_ResetRxBuffer();
    HAL_PCU_SetOutputBit(SLM320_PWRKEY_PORT, SLM320_PWRKEY_PIN, PCU_OUTPUT_BIT_CLEAR);
    SYSTICK_Wait(1500);
    HAL_PCU_SetOutputBit(SLM320_PWRKEY_PORT, SLM320_PWRKEY_PIN, PCU_OUTPUT_BIT_SET);
}

void SLM320_PowerOff(void)
{
    SLM320_ResetRxBuffer();

    SLM320_SendCmd("AT+CPOF\r\n");
    SLM320_CheckResponse("OK", 10000);

    SYSTICK_Wait(1000);

    HAL_PCU_SetOutputBit(SLM320_PWR_PORT, SLM320_PWR_PIN, PCU_OUTPUT_BIT_SET);
    HAL_PCU_SetOutputBit(SLM320_PWRKEY_PORT, SLM320_PWRKEY_PIN, PCU_OUTPUT_BIT_SET);
}

void SLM320_Reset(void)
{
    SLM320_ResetRxBuffer();

    SLM320_SendCmd("AT+TRB\r\n");
    SLM320_CheckResponse("OK", 5000);
    SYSTICK_Wait(3000);
}

static uint8_t slm320_upload_pem(const char *filename, const char *pem_data, size_t pem_len)
{
    char cmd_buf[96];
    uint32_t start;
    uint8_t got = 0U;

    snprintf(cmd_buf, sizeof(cmd_buf),
             "AT+QFUPL=\"%s\",%u,100\r\n", filename, (unsigned)pem_len);
    SLM320_ResetRxBuffer();
    SLM320_SendCmd(cmd_buf);

    start = slm320_get_tick_ms();
    while ((slm320_get_tick_ms() - start) < 5000U)
    {
        if (slm320_rx_len == 0)
            continue;
        if (slm320_memmem(slm320_rx_buf, slm320_rx_len, "CONNECT", 7) != NULL)
        {
            got = 1U;
            break;
        }
        if (slm320_memmem(slm320_rx_buf, slm320_rx_len, "+CME ERROR: 407", 15) != NULL)
        {
            got = 2U;
            break;
        }
    }

    if (got == 2U)
    {
        SLM320_ResetRxBuffer();
        return slm320_verify_cert_file(filename);
    }
    if (got != 1U)
    {
        slm320_log("[SLM320][SSL] QFUPL CONNECT not received: ");
        slm320_log(filename);
        slm320_log("\r\n");
        SLM320_ResetRxBuffer();
        return 0U;
    }

    SLM320_ResetRxBuffer();
    SLM320_SendCmd(pem_data);
    if (!SLM320_CheckResponse("+QFUPL:", 15000))
    {
        slm320_log("[SLM320][SSL] QFUPL upload failed: ");
        slm320_log(filename);
        slm320_log("\r\n");
        SLM320_ResetRxBuffer();
        return 0U;
    }

    SLM320_ResetRxBuffer();
    return slm320_verify_cert_file(filename);
}

static uint8_t slm320_verify_cert_file(const char *filename)
{
    char cmd_buf[64];

    snprintf(cmd_buf, sizeof(cmd_buf), "AT+QFLST=\"%s\"\r\n", filename);
    SLM320_ResetRxBuffer();
    SLM320_SendCmd(cmd_buf);
    if (!SLM320_CheckResponse("+QFLST:", 5000))
    {
        slm320_log("[SLM320][SSL] File not found: ");
        slm320_log(filename);
        slm320_log("\r\n");
        SLM320_ResetRxBuffer();
        return 0U;
    }

    SLM320_ResetRxBuffer();
    return 1U;
}

#if MQTT_USE_TLS_CERTS
uint8_t SLM320_CertsVerifyAll(void)
{
    if (!slm320_verify_cert_file("cacert.pem"))
        return 0U;
    if (!slm320_verify_cert_file("client.pem"))
        return 0U;
    if (!slm320_verify_cert_file("user_key.pem"))
        return 0U;
    return 1U;
}

uint8_t SLM320_CertsUploadAll(void)
{
    SLM320_ResetRxBuffer();
    SLM320_SendCmd("AT+QFDEL=\"cacert.pem\"\r\n");
    SLM320_CheckResponse("OK", 3000);
    SLM320_ResetRxBuffer();

    SLM320_SendCmd("AT+QFDEL=\"client.pem\"\r\n");
    SLM320_CheckResponse("OK", 3000);
    SLM320_ResetRxBuffer();

    SLM320_SendCmd("AT+QFDEL=\"user_key.pem\"\r\n");
    SLM320_CheckResponse("OK", 3000);
    SLM320_ResetRxBuffer();

    if (!slm320_upload_pem("cacert.pem", MqttCerts_GetRootCA(), MqttCerts_GetRootCALen()))
        return 0U;
    if (!slm320_upload_pem("client.pem", MqttCerts_GetClientCert(), MqttCerts_GetClientCertLen()))
        return 0U;
    if (!slm320_upload_pem("user_key.pem", MqttCerts_GetPrivateKey(), MqttCerts_GetPrivateKeyLen()))
        return 0U;

    return 1U;
}

uint8_t SLM320_BootstrapAt(void)
{
    uint32_t start;
    uint8_t  i;
    uint8_t  at_ok = 0U;

    SLM320_PowerOn();

    start = slm320_get_tick_ms();
    while ((slm320_get_tick_ms() - start) < 15000U)
    {
        if (slm320_rx_len > 0U &&
            (slm320_memmem(slm320_rx_buf, slm320_rx_len, "RDY", 3) != NULL ||
             slm320_memmem(slm320_rx_buf, slm320_rx_len, "AT READY", 8) != NULL))
        {
            break;
        }
    }

    for (i = 0U; i < 10U; i++)
    {
        SLM320_ResetRxBuffer();
        SLM320_SendCmd("AT\r\n");
        if (SLM320_CheckResponse("OK", 3000))
        {
            at_ok = 1U;
            break;
        }
        SYSTICK_Wait(500);
    }

    if (at_ok == 0U)
        return 0U;

    SLM320_ResetRxBuffer();
    SLM320_SendCmd("ATE0\r\n");
    SLM320_CheckResponse("OK", 2000);
    SLM320_ResetRxBuffer();
    return 1U;
}
#endif /* MQTT_USE_TLS_CERTS */

static uint8_t slm320_qmt_send_cfg(const char *cmd)
{
    SLM320_ResetRxBuffer();
    SLM320_SendCmd(cmd);
    if (!SLM320_CheckResponse("OK", 5000))
    {
        slm320_log("[SLM320] QMTCFG failed\r\n");
        SLM320_ResetRxBuffer();
        return 0U;
    }
    SLM320_ResetRxBuffer();
    return 1U;
}

static void slm320_mqtt_disconnect_prep(void)
{
#if MQTT_USE_TLS_CERTS
    SLM320_ResetRxBuffer();
    SLM320_SendCmd("AT+QMTDISC=0\r\n");
    SLM320_CheckResponse("OK", 5000);
    SLM320_ResetRxBuffer();
    SLM320_SendCmd("AT+QMTCLOSE=0\r\n");
    SLM320_CheckResponse("OK", 5000);
#else
    SLM320_ResetRxBuffer();
    SLM320_SendCmd("AT+MQTTDISCONN\r\n");
    SLM320_CheckResponse("OK", 5000);
#endif
    SLM320_ResetRxBuffer();
}

static uint8_t slm320_ssl_send_cmd(const char *cmd)
{
    SLM320_ResetRxBuffer();
    SLM320_SendCmd(cmd);
    if (!SLM320_CheckResponse("OK", 5000))
    {
        slm320_log("[SLM320] QSSLCFG failed\r\n");
        SLM320_ResetRxBuffer();
        return 0U;
    }
    SLM320_ResetRxBuffer();
    return 1U;
}

static uint8_t slm320_configure_dns(void)
{
    char cmd_buf[96];

    snprintf(cmd_buf, sizeof(cmd_buf),
             "AT+QIDNSCFG=%u,\"8.8.8.8\",\"1.1.1.1\"\r\n",
             (unsigned)CELLULAR_PDP_CONTEXT);
    SLM320_ResetRxBuffer();
    SLM320_SendCmd(cmd_buf);
    if (!SLM320_CheckResponse("OK", 5000))
    {
        SLM320_ResetRxBuffer();
        return 0U;
    }

    SLM320_ResetRxBuffer();
    return 1U;
}

static uint8_t slm320_dns_resolve(const char *host)
{
    char cmd_buf[128];
    uint32_t start;
    uint8_t got_ok = 0U;

    snprintf(cmd_buf, sizeof(cmd_buf),
             "AT+QIDNSGIP=%u,\"%s\"\r\n",
             (unsigned)CELLULAR_PDP_CONTEXT, host);
    SLM320_ResetRxBuffer();
    SLM320_SendCmd(cmd_buf);

    start = slm320_get_tick_ms();
    while ((slm320_get_tick_ms() - start) < 65000U)
    {
        if (slm320_rx_len > 0U)
        {
            if (!got_ok && slm320_memmem(slm320_rx_buf, slm320_rx_len, "OK", 2) != NULL)
                got_ok = 1U;

            if (slm320_memmem(slm320_rx_buf, slm320_rx_len, "+QIURC: \"dnsgip\"", 16) != NULL ||
                slm320_memmem(slm320_rx_buf, slm320_rx_len, "+QIURC: \"dnsgip\",", 17) != NULL)
            {
                SLM320_ResetRxBuffer();
                return 1U;
            }

            if (slm320_memmem(slm320_rx_buf, slm320_rx_len, "+CME ERROR", 10) != NULL ||
                (slm320_memmem(slm320_rx_buf, slm320_rx_len, "ERROR", 5) != NULL &&
                 slm320_memmem(slm320_rx_buf, slm320_rx_len, "+QIURC", 6) == NULL))
            {
                break;
            }
        }

        SYSTICK_Wait(100);
    }

    SLM320_ResetRxBuffer();
    return 0U;
}

static void slm320_log_rx_error(const char *stage)
{
    slm320_log("[SLM320] ");
    slm320_log(stage);
    slm320_log(" failed\r\n");
    SLM320_ResetRxBuffer();
}

SLM320_Status_t SLM320_RunStateMachine(void)
{
    char cmd_buf[256];

    switch (slm320_state)
    {
    case SLM320_STATE_IDLE:
        slm320_state = SLM320_STATE_POWER_ENABLE;
        return SLM320_OK;

    case SLM320_STATE_POWER_ENABLE:
    {
        SLM320_ResetRxBuffer();
        SLM320_SendCmd("AT\r\n");
        if (SLM320_CheckResponse("OK", 1500))
        {
            slm320_mqtt_disconnect_prep();
#if MQTT_USE_TLS_CERTS
            /* Modem already on (e.g. after cert bootstrap) — still run full
             * network + SSL setup; skipping to MQTT_OPEN breaks QMTCONN. */
            slm320_state = SLM320_STATE_CHECK_CPIN;
#else
            slm320_state = SLM320_STATE_MQTT_CONNECT;
#endif
            return SLM320_OK;
        }

        HAL_PCU_SetOutputBit(SLM320_PWR_PORT,    SLM320_PWR_PIN,    PCU_OUTPUT_BIT_SET);
        HAL_PCU_SetOutputBit(SLM320_PWRKEY_PORT, SLM320_PWRKEY_PIN, PCU_OUTPUT_BIT_SET);
        SYSTICK_Wait(2000);

        SLM320_PowerOn();
        slm320_state = SLM320_STATE_CHECK_READY;
        return SLM320_OK;
    }

    case SLM320_STATE_CHECK_READY:
    {
        if (!SLM320_CheckResponse("AT READY", 12000))
            (void)SLM320_CheckResponse("RDY", 3000);

        SLM320_ResetRxBuffer();
        slm320_state = SLM320_STATE_CHECK_AT;
        return SLM320_OK;
    }

    case SLM320_STATE_CHECK_AT:
    {
        const uint8_t MAX_AT_TRIES = 10U;
        uint8_t at_ok = 0U;

        for (uint8_t i = 0; i < MAX_AT_TRIES && !at_ok; i++)
        {
            SLM320_ResetRxBuffer();
            SLM320_SendCmd("AT\r\n");
            if (SLM320_CheckResponse("OK", 3000))
                at_ok = 1U;
            else
                SYSTICK_Wait(2000);
        }

        if (at_ok)
        {
            SLM320_ResetRxBuffer();
            SLM320_SendCmd("ATE0\r\n");
            SLM320_CheckResponse("OK", 2000);
            SLM320_ResetRxBuffer();
            slm320_state = SLM320_STATE_CHECK_CPIN;
            return SLM320_OK;
        }

        slm320_log("[SLM320] AT failed, resetting modem\r\n");
        SLM320_ResetRxBuffer();
        slm320_state = SLM320_STATE_RESET;
        return SLM320_OK;
    }

    case SLM320_STATE_CHECK_CPIN:
        {
            uint8_t cpin_ok = 0U;

            for (uint8_t tries = 0; tries < 5U; tries++)
            {
                SLM320_ResetRxBuffer();
                SLM320_SendCmd("AT+CPIN?\r\n");
                SLM320_CheckResponse("READY", 5000);

                if (strstr((char *)slm320_rx_buf, "READY") != NULL)
                {
                    cpin_ok = 1U;
                    break;
                }
                SYSTICK_Wait(2000);
            }

            if (cpin_ok)
            {
                slm320_log("[SLM320] SIM ready\r\n");
                SLM320_ResetRxBuffer();
                slm320_state = SLM320_STATE_CHECK_CREG;
                return SLM320_OK;
            }

            slm320_log("[SLM320] SIM not ready, resetting modem\r\n");
            SLM320_ResetRxBuffer();
            slm320_state = SLM320_STATE_RESET;
            return SLM320_OK;
        }

        case SLM320_STATE_CHECK_CREG:
        {
            uint32_t creg_start = slm320_get_tick_ms();
            uint8_t  registered = 0U;

            SLM320_ResetRxBuffer();
            SLM320_SendCmd("AT+CREG=0\r\n");
            SLM320_CheckResponse("OK", 2000);

            while ((slm320_get_tick_ms() - creg_start) < 60000U)
                    {
                        const char *p;
                        SLM320_ResetRxBuffer();
                        SLM320_SendCmd("AT+CREG?\r\n");
                        SLM320_CheckResponse("+CREG:", 3000);

                        SYSTICK_Wait(100);

                        p = strstr((char *)slm320_rx_buf, "+CREG:");
                        if (p != NULL && (strstr(p, ",1") != NULL || strstr(p, ",5") != NULL))
                        {
                            registered = 1U;
                            break;
                        }
                        SYSTICK_Wait(2000);
                    }

            if (registered)
            {
                slm320_log("[SLM320] Registered on network\r\n");
                SLM320_ResetRxBuffer();
                slm320_state = SLM320_STATE_SET_APN;
                return SLM320_OK;
            }

            slm320_log("[SLM320] Network registration timeout\r\n");
            SLM320_ResetRxBuffer();
            slm320_state = SLM320_STATE_RESET;
            return SLM320_OK;
        }

    case SLM320_STATE_SET_APN:
        snprintf(cmd_buf, sizeof(cmd_buf),
                 "AT+QICSGP=%d,1,\"%s\",\"%s\",\"%s\",%d\r\n",
                 CELLULAR_PDP_CONTEXT, CELLULAR_APN,
                 CELLULAR_APN_USER, CELLULAR_APN_PASS, CELLULAR_APN_AUTH);
        SLM320_ResetRxBuffer();
        SLM320_SendCmd(cmd_buf);
        if (SLM320_CheckResponse("OK", 5000))
        {
            slm320_log("[SLM320] APN configured\r\n");
            SLM320_ResetRxBuffer();
            slm320_state = SLM320_STATE_ENABLE_QIACT;
            return SLM320_OK;
        }
        slm320_log("[SLM320] APN configuration failed\r\n");
        SLM320_ResetRxBuffer();
        return SLM320_ERROR;

    case SLM320_STATE_ENABLE_QIACT:
    {
        uint8_t qiact_ok = 0U;

        SLM320_ResetRxBuffer();
        SLM320_SendCmd("AT+QIACT?\r\n");
        SLM320_CheckResponse("OK", 5000);

        if (strstr((char *)slm320_rx_buf, "+QIACT: 1,1") != NULL)
        {
            SLM320_ResetRxBuffer();
            (void)slm320_configure_dns();
#if MQTT_USE_TLS_CERTS
            slm320_state = SLM320_STATE_SSL_CFG;
#else
            slm320_state = SLM320_STATE_MQTT_CONNECT;
#endif
            return SLM320_OK;
        }

        SLM320_ResetRxBuffer();
        SLM320_SendCmd("AT+QIDEACT=1\r\n");
        SLM320_CheckResponse("OK", 40000);
        SYSTICK_Wait(1000);

        SLM320_ResetRxBuffer();
        SLM320_SendCmd("AT+QIACT=1\r\n");

        {
            uint32_t qiact_t = slm320_get_tick_ms();
            while ((slm320_get_tick_ms() - qiact_t) < 150000U)
            {
                if (slm320_rx_len == 0)
                    continue;
                if (strstr((char *)slm320_rx_buf, "OK") != NULL)
                {
                    qiact_ok = 1U;
                    break;
                }
                if (strstr((char *)slm320_rx_buf, "ERROR") != NULL)
                    break;
            }
        }

        SLM320_ResetRxBuffer();
        if (qiact_ok)
        {
            SLM320_ResetRxBuffer();
            SLM320_SendCmd("AT+QIACT?\r\n");
            SLM320_CheckResponse("+QIACT:", 5000);
            slm320_log("[SLM320] PDP context active\r\n");
            SLM320_ResetRxBuffer();

            (void)slm320_configure_dns();

#if MQTT_USE_TLS_CERTS
            slm320_state = SLM320_STATE_SSL_CFG;
#else
            slm320_state = SLM320_STATE_MQTT_CONNECT;
#endif
            return SLM320_OK;
        }

        slm320_log("[SLM320] QIACT failed, resetting modem\r\n");
        slm320_state = SLM320_STATE_RESET;
        return SLM320_OK;
    }

#if MQTT_USE_TLS_CERTS
    case SLM320_STATE_SSL_CFG:
    {
        if (MqttCerts_HasEmbedded() != 0U)
        {
            if (!SLM320_CertsUploadAll())
                return SLM320_ERROR;
        }
        else
        {
            if (!SLM320_CertsVerifyAll())
            {
                slm320_log("[SLM320][SSL] Modem cert files missing\r\n");
                return SLM320_ERROR;
            }
        }

        snprintf(cmd_buf, sizeof(cmd_buf),
                 "AT+QSSLCFG=\"cacert\",%u,\"cacert.pem\"\r\n",
                 (unsigned)SLM320_SSL_CTX_ID);
        if (!slm320_ssl_send_cmd(cmd_buf))
            return SLM320_ERROR;

        snprintf(cmd_buf, sizeof(cmd_buf),
                 "AT+QSSLCFG=\"clientcert\",%u,\"client.pem\"\r\n",
                 (unsigned)SLM320_SSL_CTX_ID);
        if (!slm320_ssl_send_cmd(cmd_buf))
            return SLM320_ERROR;

        snprintf(cmd_buf, sizeof(cmd_buf),
                 "AT+QSSLCFG=\"clientkey\",%u,\"user_key.pem\"\r\n",
                 (unsigned)SLM320_SSL_CTX_ID);
        if (!slm320_ssl_send_cmd(cmd_buf))
            return SLM320_ERROR;

        snprintf(cmd_buf, sizeof(cmd_buf),
                 "AT+QSSLCFG=\"seclevel\",%u,2\r\n",
                 (unsigned)SLM320_SSL_CTX_ID);
        if (!slm320_ssl_send_cmd(cmd_buf))
            return SLM320_ERROR;

        snprintf(cmd_buf, sizeof(cmd_buf),
                 "AT+QSSLCFG=\"sslversion\",%u,4\r\n",
                 (unsigned)SLM320_SSL_CTX_ID);
        if (!slm320_ssl_send_cmd(cmd_buf))
            return SLM320_ERROR;

        snprintf(cmd_buf, sizeof(cmd_buf),
                 "AT+QSSLCFG=\"ciphersuite\",%u,0xFFFF\r\n",
                 (unsigned)SLM320_SSL_CTX_ID);
        if (!slm320_ssl_send_cmd(cmd_buf))
            return SLM320_ERROR;

        snprintf(cmd_buf, sizeof(cmd_buf),
                 "AT+QSSLCFG=\"ignorelocaltime\",%u,1\r\n",
                 (unsigned)SLM320_SSL_CTX_ID);
        if (!slm320_ssl_send_cmd(cmd_buf))
            return SLM320_ERROR;

        snprintf(cmd_buf, sizeof(cmd_buf),
                 "AT+QSSLCFG=\"sni\",%u,1\r\n",
                 (unsigned)SLM320_SSL_CTX_ID);
        if (!slm320_ssl_send_cmd(cmd_buf))
            return SLM320_ERROR;

        snprintf(cmd_buf, sizeof(cmd_buf),
                 "AT+QSSLCFG=\"negotiatetime\",%u,120\r\n",
                 (unsigned)SLM320_SSL_CTX_ID);
        if (!slm320_ssl_send_cmd(cmd_buf))
            return SLM320_ERROR;

        snprintf(cmd_buf, sizeof(cmd_buf),
                 "AT+QMTCFG=\"ssl\",%u,1,%u\r\n",
                 (unsigned)SLM320_MQTT_CLIENT_ID, (unsigned)SLM320_SSL_CTX_ID);
        if (!slm320_qmt_send_cfg(cmd_buf))
            return SLM320_ERROR;

        snprintf(cmd_buf, sizeof(cmd_buf),
                 "AT+QMTCFG=\"version\",%u,4\r\n",
                 (unsigned)SLM320_MQTT_CLIENT_ID);
        if (!slm320_qmt_send_cfg(cmd_buf))
            return SLM320_ERROR;

        snprintf(cmd_buf, sizeof(cmd_buf),
                 "AT+QMTCFG=\"session\",%u,1\r\n",
                 (unsigned)SLM320_MQTT_CLIENT_ID);
        if (!slm320_qmt_send_cfg(cmd_buf))
            return SLM320_ERROR;

        snprintf(cmd_buf, sizeof(cmd_buf),
                 "AT+QMTCFG=\"keepalive\",%u,%u\r\n",
                 (unsigned)SLM320_MQTT_CLIENT_ID, (unsigned)MQTT_KEEP_ALIVE);
        if (!slm320_qmt_send_cfg(cmd_buf))
            return SLM320_ERROR;

        slm320_log("[SLM320] TLS ready\r\n");
        slm320_state = SLM320_STATE_MQTT_OPEN;
        return SLM320_OK;
    }
#endif /* MQTT_USE_TLS_CERTS */

#if MQTT_USE_TLS_CERTS
    case SLM320_STATE_MQTT_OPEN:
    {
        uint8_t open_ok = 0U;
        const uint8_t MAX_RETRIES = 3U;

        for (uint8_t attempt = 0U; attempt < MAX_RETRIES && !open_ok; attempt++)
        {
            if (attempt > 0U)
            {
                slm320_mqtt_disconnect_prep();
                SYSTICK_Wait(2000);
            }

            snprintf(cmd_buf, sizeof(cmd_buf),
                     "AT+QMTOPEN=%u,\"%s\",%u\r\n",
                     (unsigned)SLM320_MQTT_CLIENT_ID,
                     MQTT_BROKER_HOST,
                     (unsigned)MQTT_BROKER_PORT);
            SLM320_ResetRxBuffer();
            SLM320_SendCmd(cmd_buf);
            if (!SLM320_CheckResponse("OK", 5000))
                continue;

            {
                uint32_t urc_start = slm320_get_tick_ms();
                while ((slm320_get_tick_ms() - urc_start) < 60000U)
                {
                    if (slm320_rx_len == 0U)
                        continue;
                    if (slm320_memmem(slm320_rx_buf, slm320_rx_len, "+QMTOPEN:", 9) != NULL)
                    {
                        if (slm320_memmem(slm320_rx_buf, slm320_rx_len, "+QMTOPEN: 0,0", 13) != NULL)
                            open_ok = 1U;
                        break;
                    }
                    SYSTICK_Wait(100);
                }
            }
            SLM320_ResetRxBuffer();
        }

        if (open_ok)
        {
            slm320_log("[SLM320] MQTT broker open\r\n");
            slm320_state = SLM320_STATE_MQTT_CONNECT;
            return SLM320_OK;
        }

        slm320_log("[SLM320] MQTT open failed\r\n");
        slm320_state = SLM320_STATE_RESET;
        return SLM320_OK;
    }
#endif /* MQTT_USE_TLS_CERTS */

    case SLM320_STATE_MQTT_CONNECT:
    {
        uint8_t mqtt_ok = 0U;
        const uint8_t MAX_RETRIES = 3U;

#if MQTT_USE_TLS_CERTS
        for (uint8_t attempt = 0U; attempt < MAX_RETRIES && !mqtt_ok; attempt++)
        {
            if (attempt > 0U)
            {
                slm320_mqtt_disconnect_prep();
                SYSTICK_Wait(2000);
                slm320_state = SLM320_STATE_MQTT_OPEN;
                return SLM320_OK;
            }

            snprintf(cmd_buf, sizeof(cmd_buf),
                     "AT+QMTCONN=%u,\"%s\"\r\n",
                     (unsigned)SLM320_MQTT_CLIENT_ID, MQTT_CLIENT_ID);
            SLM320_ResetRxBuffer();
            SLM320_SendCmd(cmd_buf);

            {
                uint32_t conn_start = slm320_get_tick_ms();
                uint8_t got_urc = 0U;
                while ((slm320_get_tick_ms() - conn_start) < 60000U)
                {
                    if (slm320_rx_len == 0U)
                        continue;
                    if (slm320_memmem(slm320_rx_buf, slm320_rx_len, "+QMTCONN:", 9) != NULL)
                    {
                        got_urc = 1U;
                        break;
                    }
                    if (slm320_memmem(slm320_rx_buf, slm320_rx_len, "+QMTSTAT:", 9) != NULL)
                        break;
                    SYSTICK_Wait(100);
                }

                if (got_urc)
                {
                    if (slm320_memmem(slm320_rx_buf, slm320_rx_len, "+QMTCONN: 0,0,0", 15) != NULL ||
                        slm320_memmem(slm320_rx_buf, slm320_rx_len, "+QMTCONN: 0,0\r", 14) != NULL)
                        mqtt_ok = 1U;
                }
                else
                    slm320_log_rx_error("QMTCONN");
            }
            SLM320_ResetRxBuffer();
        }
#else
        for (uint8_t attempt = 0U; attempt < MAX_RETRIES && !mqtt_ok; attempt++)
        {
            if (attempt > 0U)
            {
                slm320_mqtt_disconnect_prep();
                SYSTICK_Wait(2000);
            }

            snprintf(cmd_buf, sizeof(cmd_buf),
                     "AT+MQTTCONN=\"%s\",%u,\"%s\",%u,1\r\n",
                     MQTT_BROKER_HOST,
                     (unsigned)MQTT_BROKER_PORT,
                     MQTT_CLIENT_ID,
                     (unsigned)MQTT_KEEP_ALIVE);
            SLM320_ResetRxBuffer();
            SLM320_SendCmd(cmd_buf);

            {
                uint32_t conn_start = slm320_get_tick_ms();
                while ((slm320_get_tick_ms() - conn_start) < 60000U)
                {
                    if (slm320_rx_len == 0U)
                        continue;

                    if (slm320_memmem(slm320_rx_buf, slm320_rx_len, "OK", 2) != NULL &&
                        slm320_memmem(slm320_rx_buf, slm320_rx_len, "+MQTTDISCONNECTED", 17) == NULL)
                    {
                        mqtt_ok = 1U;
                        break;
                    }
                    if (slm320_memmem(slm320_rx_buf, slm320_rx_len, "ERROR", 5) != NULL ||
                        slm320_memmem(slm320_rx_buf, slm320_rx_len, "+MQTTDISCONNECTED", 17) != NULL)
                    {
                        slm320_log_rx_error("MQTTCONN");
                        break;
                    }
                    SYSTICK_Wait(100);
                }
            }
            SLM320_ResetRxBuffer();
        }
#endif /* MQTT_USE_TLS_CERTS */

        if (mqtt_ok)
        {
            SLM320_ResetRxBuffer();
            slm320_state = SLM320_STATE_GET_IMEI;
            return SLM320_OK;
        }

        slm320_log("[SLM320] MQTT connect failed\r\n");
        SLM320_ResetRxBuffer();
        slm320_state = SLM320_STATE_RESET;
        return SLM320_OK;
    }

    case SLM320_STATE_GET_IMEI:
    {
        if (SLM320_GetIMEI() != SLM320_OK)
            snprintf(slm320_imei, sizeof(slm320_imei), "unknown");
        slm320_state = SLM320_STATE_MQTT_PUBLISH;
        return SLM320_OK;
    }

    case SLM320_STATE_MQTT_PUBLISH:
        return SLM320_OK;

    case SLM320_STATE_MQTT_DISCONNECT:
        slm320_mqtt_disconnect_prep();
        slm320_state = SLM320_STATE_POWER_OFF;
        return SLM320_OK;

    case SLM320_STATE_POWER_OFF:
        SLM320_PowerOff();
        slm320_state = SLM320_STATE_DONE;
        return SLM320_OK;

    case SLM320_STATE_RESET:
        slm320_log("[SLM320] Resetting modem\r\n");
        SLM320_PowerOff();
        SYSTICK_Wait(500);
        SLM320_PowerOn();
        slm320_state = SLM320_STATE_CHECK_READY;
        return SLM320_OK;

    case SLM320_STATE_DONE:
        return SLM320_OK;

    case SLM320_STATE_ERROR:
        slm320_log("[SLM320] Error state\r\n");
        return SLM320_ERROR;

    default:
        return SLM320_ERROR;
    }
}

SLM320_Status_t SLM320_PublishSensorData(const char *topic, const char *data)
{
    char cmd_buf[900];

    if (topic == NULL || data == NULL)
        return SLM320_ERROR;

    size_t payload_len = strlen(data);
    if (payload_len == 0U || payload_len > SLM320_MQTT_MSG_MAX)
        return SLM320_ERROR;

#if MQTT_USE_TLS_CERTS
    snprintf(cmd_buf, sizeof(cmd_buf),
             "AT+QMTPUBEX=%u,0,0,0,\"%s\",%u\r\n",
             (unsigned)SLM320_MQTT_CLIENT_ID, topic, (unsigned)payload_len);

    SLM320_ResetRxBuffer();
    SLM320_SendCmd(cmd_buf);

    if (!SLM320_CheckResponse(">", 10000))
    {
        slm320_log("[SLM320] Publish prompt timeout\r\n");
        SLM320_ResetRxBuffer();
        return SLM320_ERROR;
    }

    SLM320_ResetRxBuffer();
    SYSTICK_Wait(50);
    HAL_UART_Transmit(SLM320_UART_ID, (uint8_t *)data, (uint32_t)payload_len, true);

    {
        uint32_t pub_start = slm320_get_tick_ms();
        while ((slm320_get_tick_ms() - pub_start) < 30000U)
        {
            if (slm320_rx_len == 0U)
                continue;
            if (slm320_memmem(slm320_rx_buf, slm320_rx_len, "+QMTPUBEX:", 10) != NULL)
                break;
            if (slm320_memmem(slm320_rx_buf, slm320_rx_len, "ERROR", 5) != NULL)
                break;
            SYSTICK_Wait(50);
        }
    }

    if (slm320_memmem(slm320_rx_buf, slm320_rx_len, "+QMTPUBEX: 0,0,0", 16) != NULL ||
        slm320_memmem(slm320_rx_buf, slm320_rx_len, "+QMTPUBEX: 0,0,1", 16) != NULL)
    {
        SLM320_ResetRxBuffer();
        return SLM320_OK;
    }
#else
    snprintf(cmd_buf, sizeof(cmd_buf),
             "AT+MQTTPUB=\"%s\",\"%s\",1,0,0\r\n",
             topic, data);

    SLM320_ResetRxBuffer();
    SLM320_SendCmd(cmd_buf);

    if (SLM320_CheckResponse("OK", 60000))
    {
        SLM320_ResetRxBuffer();
        return SLM320_OK;
    }
#endif

    slm320_log("[SLM320] Publish failed\r\n");
    SLM320_ResetRxBuffer();
    return SLM320_ERROR;
}

SLM320_Status_t SLM320_GetIMEI(void)
{
    SLM320_ResetRxBuffer();
    SLM320_SendCmd("AT+GSN\r\n");

    if (SLM320_CheckResponse("OK", 3000))
    {
        char *p = (char *)slm320_rx_buf;
        char *end;

        while (*p != '\0' && (*p < '0' || *p > '9'))
            p++;
        end = p;
        while (*end >= '0' && *end <= '9')
            end++;

        uint8_t imei_len = (uint8_t)(end - p);
        if (imei_len == 15U)
        {
            memcpy(slm320_imei, p, 15U);
            slm320_imei[15] = '\0';
            SLM320_ResetRxBuffer();
            return SLM320_OK;
        }
    }
    SLM320_ResetRxBuffer();
    return SLM320_ERROR;
}

void SLM320_UART_RxCallback(void)
{
    if (slm320_rx_len < SLM320_RX_BUF_SIZE)
    {
        slm320_rx_buf[slm320_rx_len] = s_rx_byte;
        slm320_rx_len++;
    }
    HAL_UART_Receive(SLM320_UART_ID, &s_rx_byte, 1, false);
}

static void *slm320_memmem(const void *haystack, size_t haystacklen,
                           const void *needle, size_t needlelen)
{
    if (needlelen == 0)
        return (void *)haystack;
    if (haystacklen < needlelen)
        return NULL;

    const uint8_t *h = (const uint8_t *)haystack;
    for (size_t i = 0; i <= haystacklen - needlelen; i++)
    {
        if (h[i] == ((const uint8_t *)needle)[0])
        {
            if (memcmp(&h[i], needle, needlelen) == 0)
                return (void *)&h[i];
        }
    }
    return NULL;
}
