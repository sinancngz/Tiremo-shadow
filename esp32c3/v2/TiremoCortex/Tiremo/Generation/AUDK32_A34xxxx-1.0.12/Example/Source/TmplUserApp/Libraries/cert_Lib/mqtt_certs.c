/**
 * @file    mqtt_certs.c
 * @brief   TLS certificate accessors — isolated .cert_flash section, erasable at runtime
 *
 * Certificates live in their own linker section (.cert_flash) which occupies
 * dedicated, page-aligned code-flash pages.  After uploading to ESP32 the
 * pages are erased via direct CFMC register access (no HAL_FMC module
 * required) so that private key material no longer exists on the MCU.  On the next boot the first-byte check (0xFF = erased) detects
 * "no certificates" and the firmware proceeds straight to MQTT connect.
 * Re-flashing the MCU via IDE restores the section and triggers a new
 * provisioning cycle automatically.
 */

#include "mqtt_certs.h"
#include "../DebugLibrary/debug_framework.h"

#include <stdint.h>
#include <string.h>
#include <stdio.h>

#if MQTT_USE_TLS_CERTS

#include "abov_config.h"

#include "../../certificates/mqtt_rootCA.inc"
#include "../../certificates/mqtt_certificate.inc"
#include "../../certificates/mqtt_private.inc"

/* ---- linker symbols ---------------------------------------------------- */
extern const uint32_t __cert_flash_start__;
extern const uint32_t __cert_flash_end__;

/* ---- constants --------------------------------------------------------- */
#define CERT_PAGE_SIZE          512U
#define CERT_ROOT_BUF_SIZE     1408U
#define CERT_CLIENT_BUF_SIZE   1408U
#define CERT_KEY_BUF_SIZE      1792U
#define CERT_TITLE_LEN           64U

/* CFMC page erase — self-contained, no HAL_FMC dependency (MCUbrew-safe) */
#define CERT_FMC_PE_CMD1        0x01234567U
#define CERT_FMC_PE_CMD2        0x12345678U
#define CERT_FMC_PE_CMD3        0x23456789U
#define CERT_FMC_BUSY_MAX       0x35000U
#define CERT_FMC_STAT_ERR_MSK   0x003F0000U

/* ---- slot table -------------------------------------------------------- */
typedef struct
{
    const char *name;
    const char *flash_src;
    char       *buf;
    size_t      cap;
    size_t      len;
} CertSlot_t;

static char s_buf_root  [CERT_ROOT_BUF_SIZE];
static char s_buf_client[CERT_CLIENT_BUF_SIZE];
static char s_buf_key   [CERT_KEY_BUF_SIZE];

static CertSlot_t s_slots[] =
{
    { "mqtt_rootCA",      s_mqtt_root_ca,     s_buf_root,   CERT_ROOT_BUF_SIZE,   0U },
    { "mqtt_certificate", s_mqtt_client_cert, s_buf_client, CERT_CLIENT_BUF_SIZE, 0U },
    { "mqtt_private_key", s_mqtt_private_key, s_buf_key,    CERT_KEY_BUF_SIZE,    0U },
};

static uint8_t s_loaded = 0U;

/* ---- helpers ----------------------------------------------------------- */

static uint8_t cert_pem_valid(const char *p)
{
    return ((uint8_t)p[0] != 0xFFU && p[0] == '-') ? 1U : 0U;
}

static size_t cert_strnlen(const char *s, size_t max)
{
    size_t n = 0U;
    while (n < max && s[n] != '\0')
        n++;
    return n;
}

/* ---- load -------------------------------------------------------------- */

static void mqtt_certs_load(void)
{
    uint8_t i;

    if (s_loaded != 0U)
        return;

    for (i = 0U; i < 3U; i++)
    {
        const char *src = s_slots[i].flash_src;

        if (cert_pem_valid(src) == 0U)
        {
            memset(s_slots[i].buf, 0, s_slots[i].cap);
            s_slots[i].len = 0U;
            continue;
        }

        size_t srcLen = cert_strnlen(src, s_slots[i].cap - 1U);
        if (srcLen == 0U)
        {
            memset(s_slots[i].buf, 0, s_slots[i].cap);
            s_slots[i].len = 0U;
            continue;
        }

        memcpy(s_slots[i].buf, src, srcLen);
        s_slots[i].buf[srcLen] = '\0';
        s_slots[i].len = srcLen;
    }

    s_loaded = 1U;
}

/* ---- public accessors -------------------------------------------------- */

const char *MqttCerts_GetRootCA(void)
{
    mqtt_certs_load();
    return (s_slots[0].len > 0U) ? s_slots[0].buf : "";
}

const char *MqttCerts_GetClientCert(void)
{
    mqtt_certs_load();
    return (s_slots[1].len > 0U) ? s_slots[1].buf : "";
}

const char *MqttCerts_GetPrivateKey(void)
{
    mqtt_certs_load();
    return (s_slots[2].len > 0U) ? s_slots[2].buf : "";
}

size_t MqttCerts_GetRootCALen(void)      { mqtt_certs_load(); return s_slots[0].len; }
size_t MqttCerts_GetClientCertLen(void)  { mqtt_certs_load(); return s_slots[1].len; }
size_t MqttCerts_GetPrivateKeyLen(void)  { mqtt_certs_load(); return s_slots[2].len; }

uint8_t MqttCerts_HasEmbedded(void)
{
    mqtt_certs_load();
    return (s_slots[0].len > 0U && s_slots[1].len > 0U && s_slots[2].len > 0U)
           ? 1U : 0U;
}

/* ---- flash erase (runs from RAM — must not execute from flash being erased) */

RAMFUNC static int cert_fmc_erase_page(uint32_t page_addr)
{
    CFMC_Type *cfmc = CFMC;
    uint32_t addr = page_addr & ~3U;
    uint32_t cnt = 0U;
    uint32_t stat;

    cfmc->CONF |= CFMC_CONF_WTEN_Msk;

    cfmc->FLSKEY = CERT_FMC_PE_CMD1;
    cfmc->FLSKEY = CERT_FMC_PE_CMD2;
    cfmc->FLSKEY = CERT_FMC_PE_CMD3;

    cfmc->CTRL |= CFMC_CTRL_PERS_Msk;
    *(volatile uint32_t *)addr = 0U;

    do
    {
        __NOP(); __NOP(); __NOP(); __NOP(); __NOP();
        cnt++;
        if (cnt > CERT_FMC_BUSY_MAX)
            break;
    } while ((cfmc->STAT & CFMC_STAT_WBUSY_Msk) != 0U);

    cfmc->CTRL &= ~CFMC_CTRL_PERS_Msk;
    cfmc->CTRL |= CFMC_CTRL_FLOCK_Msk;
    cfmc->CONF &= ~CFMC_CONF_WTEN_Msk;

    if (cnt > CERT_FMC_BUSY_MAX)
        return -1;

    stat = cfmc->STAT;
    cfmc->STAT = stat;
    if ((stat & CERT_FMC_STAT_ERR_MSK) != 0U)
        return -1;

    return 0;
}

int MqttCerts_EraseFlash(void)
{
    uint32_t start = (uint32_t)(uintptr_t)&__cert_flash_start__;
    uint32_t end   = (uint32_t)(uintptr_t)&__cert_flash_end__;
    uint32_t addr;
    uint8_t  i;

    for (i = 0U; i < 3U; i++)
    {
        memset(s_slots[i].buf, 0, s_slots[i].cap);
        s_slots[i].len = 0U;
    }

    DebugFramework_Printf(
        "[CERT] Erasing flash: 0x%08X - 0x%08X\n\r",
        (unsigned int)start, (unsigned int)end);

    for (addr = start; addr < end; addr += CERT_PAGE_SIZE)
    {
        if (cert_fmc_erase_page(addr) != 0)
        {
            DebugFramework_Printf(
                "[CERT] Page erase failed @0x%08X\n\r",
                (unsigned int)addr);
            return -1;
        }
    }

    DebugFramework_PutsLine("[CERT] Flash certificates permanently erased");
    return 0;
}

/* ---- logging ----------------------------------------------------------- */

static uint8_t cert_flash_has_data(const char *p)
{
    return ((uint8_t)p[0] != 0xFFU) ? 1U : 0U;
}

void MqttCerts_LogFlashIfPresent(void)
{
    uint8_t i;
    char title[CERT_TITLE_LEN];

    for (i = 0U; i < 3U; i++)
    {
        const CertSlot_t *slot = &s_slots[i];
        const char *src = slot->flash_src;
        size_t srcLen;
        size_t j = 0U;

        if (cert_flash_has_data(src) == 0U)
            continue;

        srcLen = cert_strnlen(src, slot->cap - 1U);

        if (cert_pem_valid(src) != 0U)
        {
            while (j < srcLen && j < (CERT_TITLE_LEN - 1U) &&
                   src[j] != '\r' && src[j] != '\n')
            {
                title[j] = src[j];
                j++;
            }
        }
        title[j] = '\0';
        if (j == 0U)
        {
            title[0] = '?';
            title[1] = '\0';
        }

        DebugFramework_Printf(
            "[CERT] FW  %-18s @0x%08X len=%u b0=0x%02X \"%s\"\n\r",
            slot->name,
            (unsigned int)(uintptr_t)src,
            (unsigned int)srcLen,
            (unsigned int)(uint8_t)src[0],
            title);
    }
}

void MqttCerts_LogStorage(const char *phase)
{
    uint8_t i;
    char title[CERT_TITLE_LEN];

    for (i = 0U; i < 3U; i++)
    {
        const CertSlot_t *slot = &s_slots[i];
        unsigned int ramLen = (unsigned int)slot->len;
        unsigned int ramB0  = (ramLen > 0U)
                              ? (unsigned int)(uint8_t)slot->buf[0] : 0U;
        size_t j = 0U;

        if (ramLen > 0U)
        {
            while (j < slot->len && j < (CERT_TITLE_LEN - 1U) &&
                   slot->buf[j] != '\r' && slot->buf[j] != '\n')
            {
                title[j] = slot->buf[j];
                j++;
            }
        }
        title[j] = '\0';
        if (j == 0U)
        {
            title[0] = '-';
            title[1] = '\0';
        }

        DebugFramework_Printf(
            "[CERT] %s %-18s RAM @0x%08X len=%u b0=0x%02X \"%s\"\n\r",
            phase, slot->name,
            (unsigned int)(uintptr_t)slot->buf,
            ramLen, ramB0, title);

        {
            unsigned int fB0 = (unsigned int)(uint8_t)slot->flash_src[0];
            DebugFramework_Printf(
                "[CERT] %s %-18s FW  @0x%08X b0=0x%02X %s\n\r",
                phase, slot->name,
                (unsigned int)(uintptr_t)slot->flash_src,
                fB0,
                cert_pem_valid(slot->flash_src) ? "VALID" : "ERASED");
        }
    }
}

#else /* MQTT_USE_TLS_CERTS disabled */

const char *MqttCerts_GetRootCA(void)      { return ""; }
const char *MqttCerts_GetClientCert(void)  { return ""; }
const char *MqttCerts_GetPrivateKey(void)  { return ""; }

size_t MqttCerts_GetRootCALen(void)      { return 0U; }
size_t MqttCerts_GetClientCertLen(void)  { return 0U; }
size_t MqttCerts_GetPrivateKeyLen(void)  { return 0U; }

uint8_t MqttCerts_HasEmbedded(void)      { return 0U; }
int     MqttCerts_EraseFlash(void)       { return 0; }
void    MqttCerts_LogStorage(const char *phase) { (void)phase; }
void    MqttCerts_LogFlashIfPresent(void)      { }

#endif /* MQTT_USE_TLS_CERTS */
