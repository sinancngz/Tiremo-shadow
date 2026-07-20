/**
 * @file    mqtt_cert_provision.c
 * @brief   Upload ABOV certificates to modem(s), then erase from code-flash
 */

#include "mqtt_cert_provision.h"
#include "mqtt_certs.h"
#include "../DebugLibrary/debug_framework.h"
#include "../UserButton/user_button.h"
#include "../../config/project_config.h"

#if defined(EMPA_ESP32_MQTT_AWS)
#include "../MQTT_Library/mqtt_core.h"
#include "../MQTT_Library/mqtt_port.h"
#endif

#if defined(EMPA_SLM320_4G)
#include "../MEIG_SLM3XX/slm320.h"
#endif

extern void SYSTICK_Wait(uint32_t un32TimeMS);

static void prov_wait_long_press(void)
{
    DebugFramework_PutsLine("[CERT] Press button for 3+ seconds to erase ABOV certificates");

    while (1)
    {
        if (UserButton_ConsumeLongPress() != 0U)
            break;

        SYSTICK_Wait(10U);
    }
}

int MqttCertProv_Run(char *buffer, uint16_t bufSize)
{
#if MQTT_USE_TLS_CERTS
    if (MqttCerts_HasEmbedded() == 0U)
        return 0;

#if defined(EMPA_ESP32_MQTT_AWS)
    (void)bufSize;

    DebugFramework_PutsLine("[CERT] Uploading ABOV certificates to ESP32...");
    if (Wifi_MqttCertsUpload2(buffer, POLLING_MODE) != FUNC_OK)
    {
        DebugFramework_PutsLine("[CERT] ESP32 upload failed");
        return -1;
    }
    DebugFramework_PutsLine("[CERT] ESP32 upload complete");
#endif

#if defined(EMPA_SLM320_4G)
    (void)buffer;
    (void)bufSize;

    DebugFramework_PutsLine("[CERT] Uploading ABOV certificates to SLM320...");
    if (SLM320_BootstrapAt() == 0U)
    {
        DebugFramework_PutsLine("[CERT] SLM320 AT not ready");
        return -1;
    }
    if (SLM320_CertsUploadAll() == 0U)
    {
        DebugFramework_PutsLine("[CERT] SLM320 upload failed");
        return -1;
    }
    DebugFramework_PutsLine("[CERT] SLM320 upload complete");
#endif

    DebugFramework_PutsLine("\n\r[CERT] --- BEFORE ---");
    MqttCerts_LogStorage("BEFORE");

    prov_wait_long_press();

    if (MqttCerts_EraseFlash() != 0)
    {
        DebugFramework_PutsLine("[CERT] Flash erase failed!");
        return -1;
    }

    DebugFramework_PutsLine("\n\r[CERT] --- AFTER ---");
    MqttCerts_LogStorage("AFTER");

#if defined(EMPA_SLM320_4G)
    /* Clear modem state after cert UFS upload + flash erase so connect
     * runs the full registration / SSL_CFG path on a clean module. */
    SLM320_Reset();
#endif

    return 0;
#else
    (void)buffer;
    (void)bufSize;
    return 0;
#endif
}
