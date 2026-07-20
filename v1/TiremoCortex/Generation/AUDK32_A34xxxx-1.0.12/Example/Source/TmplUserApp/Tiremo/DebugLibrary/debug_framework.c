/**
 *******************************************************************************
 * @file        debug_framework.c
 * @brief       Tiremo debug UART library implementation
 *
 * @details     TX: blocking HAL_UART_Transmit (reliable debug log).
 *              RX: IRQ ring buffer + optional callback (UARTn_Interrupt).
 *
 * @note        UART hardware init is owned by MCUbrew (user_uart.c).
 *
 * Copyright 2024 Tiremo. All rights reserved.
 ******************************************************************************/

#include "abov_config.h"
#include "abov_module_config.h"
#include "debug_framework.h"

#include "hal_uart.h"

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#if (TIREMO_DEBUG_RX_RING_SIZE < 2U) || ((TIREMO_DEBUG_RX_RING_SIZE & (TIREMO_DEBUG_RX_RING_SIZE - 1U)) != 0U)
#error "TIREMO_DEBUG_RX_RING_SIZE must be a power of two >= 2"
#endif

#define DBG_UART_ID     UART_ID_0

#define DBG_RX_MASK     (TIREMO_DEBUG_RX_RING_SIZE - 1U)

typedef struct
{
    volatile uint32_t head;
    volatile uint32_t tail;
    uint8_t data[TIREMO_DEBUG_RX_RING_SIZE];
} DbgRxRing_t;

static bool s_initialized = false;
static bool s_uartActive = false;
static volatile bool s_rxArmPending = false;
static DebugFramework_RxCallback_t s_rxCallback = NULL;

static DbgRxRing_t s_rxRing;
static uint8_t s_rxByte = 0U;

static bool PRV_RxRingIsFull(void)
{
    return ((s_rxRing.head - s_rxRing.tail) >= (TIREMO_DEBUG_RX_RING_SIZE - 1U));
}

static bool PRV_RxRingIsEmpty(void)
{
    return (s_rxRing.head == s_rxRing.tail);
}

static void PRV_RxRingPush(uint8_t byte)
{
    uint32_t idx;

    if (PRV_RxRingIsFull())
    {
        return;
    }

    idx = s_rxRing.head & DBG_RX_MASK;
    s_rxRing.data[idx] = byte;
    s_rxRing.head++;
}

static bool PRV_RxRingPop(uint8_t *pByte)
{
    uint32_t idx;

    if (PRV_RxRingIsEmpty())
    {
        return false;
    }

    idx = s_rxRing.tail & DBG_RX_MASK;
    *pByte = s_rxRing.data[idx];
    s_rxRing.tail++;
    return true;
}

static uint32_t PRV_RxRingCount(void)
{
    return (s_rxRing.head - s_rxRing.tail);
}

static void PRV_ArmRx(void)
{
    if (!s_uartActive)
    {
        return;
    }

    if (HAL_UART_Receive(DBG_UART_ID, &s_rxByte, 1U, false) == HAL_ERR_OK)
    {
        s_rxArmPending = false;
    }
    else
    {
        s_rxArmPending = true;
    }
}

static uint32_t PRV_Read(uint8_t *pBuffer, uint32_t maxLength)
{
    uint32_t count = 0U;
    uint32_t primask;
    uint8_t byte;

    if ((pBuffer == NULL) || (maxLength == 0U) || (!s_uartActive))
    {
        return 0U;
    }

    primask = (uint32_t)__get_PRIMASK();
    __disable_irq();

    while (count < maxLength)
    {
        if (!PRV_RxRingPop(&byte))
        {
            break;
        }
        pBuffer[count] = byte;
        count++;
    }

    if (primask == 0U)
    {
        __enable_irq();
    }

    return count;
}

static bool PRV_StartUart(void)
{
    if (s_uartActive)
    {
        return true;
    }

    s_rxRing.head = 0U;
    s_rxRing.tail = 0U;
    s_uartActive = true;
    return true;
}

static void PRV_StopUart(void)
{
    s_uartActive = false;
    (void)HAL_UART_Abort(DBG_UART_ID);
}

void DebugFramework_SetRxCallback(DebugFramework_RxCallback_t pfn)
{
    s_rxCallback = pfn;

    if (s_uartActive && s_initialized && (pfn != NULL))
    {
        __enable_irq();
        PRV_ArmRx();
    }
}

void DebugFramework_IrqHandler(uint32_t un32Event, void *pContext)
{
    (void)pContext;

    if (!s_uartActive)
    {
        return;
    }

    if ((un32Event & (uint32_t)UART_EVENT_RX_DONE) != 0U)
    {
        PRV_RxRingPush(s_rxByte);
        if (s_rxCallback != NULL)
        {
            s_rxCallback(s_rxByte);
        }
        PRV_ArmRx();
    }

    if ((un32Event & (uint32_t)UART_EVENT_LINE_ERROR) != 0U)
    {
        PRV_ArmRx();
    }

    if ((un32Event & (uint32_t)UART_EVENT_TX_DONE) != 0U)
    {
        if (s_rxArmPending)
        {
            PRV_ArmRx();
        }
    }
}

bool DebugFramework_Init(void)
{
    if (!PRV_StartUart())
    {
        return false;
    }

    s_initialized = true;
    return true;
}

void DebugFramework_Uninit(void)
{
    PRV_StopUart();
    s_rxCallback = NULL;
    s_rxArmPending = false;
    s_initialized = false;
}

static void PRV_TransmitBlocking(const uint8_t *pData, uint32_t length)
{
    if (!s_initialized || (pData == NULL) || (length == 0U))
    {
        return;
    }

    (void)HAL_UART_Transmit(DBG_UART_ID, (uint8_t *)pData, length, true);
}

static void PRV_SendByte(uint8_t data)
{
    PRV_TransmitBlocking(&data, 1U);
}

void DebugFramework_PutChar(uint8_t ch)
{
    PRV_SendByte(ch);
}

void DebugFramework_Puts(const char *str)
{
    uint32_t length;

    if ((str == NULL) || (!s_initialized))
    {
        return;
    }

    length = (uint32_t)strlen(str);
    if (length == 0U)
    {
        return;
    }

    PRV_TransmitBlocking((const uint8_t *)str, length);
}

void DebugFramework_PutsLine(const char *str)
{
    DebugFramework_Puts(str);
    DebugFramework_Puts("\r\n");
}

void DebugFramework_PutDec(uint8_t num)
{
    uint8_t c1 = (uint8_t)(num % 10U);
    uint8_t c2 = (uint8_t)((num / 10U) % 10U);
    uint8_t c3 = (uint8_t)((num / 100U) % 10U);

    PRV_SendByte((uint8_t)('0' + c3));
    PRV_SendByte((uint8_t)('0' + c2));
    PRV_SendByte((uint8_t)('0' + c1));
}

void DebugFramework_PutDec16(uint16_t num)
{
    char buffer[6];
    int i = 0;

    if (num == 0U)
    {
        PRV_SendByte((uint8_t)'0');
        return;
    }

    while ((num > 0U) && (i < 5))
    {
        buffer[i++] = (char)('0' + (num % 10U));
        num = (uint16_t)(num / 10U);
    }

    while (i > 0)
    {
        PRV_SendByte((uint8_t)buffer[--i]);
    }
}

void DebugFramework_PutDec32(uint32_t num)
{
    char buffer[11];
    int i = 0;

    if (num == 0UL)
    {
        PRV_SendByte((uint8_t)'0');
        return;
    }

    while ((num > 0UL) && (i < 10))
    {
        buffer[i++] = (char)('0' + (num % 10UL));
        num /= 10UL;
    }

    while (i > 0)
    {
        PRV_SendByte((uint8_t)buffer[--i]);
    }
}

void DebugFramework_PutHex(uint8_t num)
{
    static const char hex[] = "0123456789ABCDEF";

    PRV_SendByte((uint8_t)hex[(num >> 4) & 0x0FU]);
    PRV_SendByte((uint8_t)hex[num & 0x0FU]);
}

void DebugFramework_PutHex16(uint16_t num)
{
    DebugFramework_PutHex((uint8_t)(num >> 8));
    DebugFramework_PutHex((uint8_t)(num & 0xFFU));
}

void DebugFramework_PutHex32(uint32_t num)
{
    DebugFramework_PutHex16((uint16_t)(num >> 16));
    DebugFramework_PutHex16((uint16_t)(num & 0xFFFFUL));
}

void DebugFramework_Printf(const char *format, ...)
{
    char buffer[160];
    int len;
    va_list args;

    if ((format == NULL) || (!s_initialized))
    {
        return;
    }

    va_start(args, format);
    len = vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);

    if (len <= 0)
    {
        return;
    }

    if (len >= (int)sizeof(buffer))
    {
        len = (int)sizeof(buffer) - 1;
    }

    PRV_TransmitBlocking((const uint8_t *)buffer, (uint32_t)len);
}

uint8_t DebugFramework_WaitChar(void)
{
    uint8_t ch = 0U;
    HAL_ERR_e eErr;

    if (!s_initialized)
    {
        return 0U;
    }

    do
    {
        eErr = HAL_UART_Receive(DBG_UART_ID, &ch, 1U, true);
    }
    while (eErr != HAL_ERR_OK);

    return ch;
}

uint8_t DebugFramework_GetChar(void)
{
    uint8_t ch = 0U;

    if (!s_initialized)
    {
        return 0U;
    }

    if (PRV_Read(&ch, 1U) == 0U)
    {
        return 0U;
    }

    return ch;
}

uint32_t DebugFramework_Read(uint8_t *pBuffer, uint32_t maxLength)
{
    if (!s_initialized)
    {
        return 0U;
    }

    return PRV_Read(pBuffer, maxLength);
}

bool DebugFramework_IsDataAvailable(void)
{
    uint32_t count;
    uint32_t primask;

    if (!s_initialized)
    {
        return false;
    }

    primask = (uint32_t)__get_PRIMASK();
    __disable_irq();
    count = PRV_RxRingCount();
    if (primask == 0U)
    {
        __enable_irq();
    }

    return (count > 0U);
}

bool DebugFramework_IsInitialized(void)
{
    return s_initialized;
}
