/**
 *******************************************************************************
 * @file        debug_framework.h
 * @brief       Tiremo debug UART library — public API
 *
 * @details     Debug channel on UART0 @ 115200 (MCUbrew: user_uart.c).
 *              TX: blocking HAL_UART_Transmit.
 *              RX IRQ: SetRxCallback + IrqHandler (UART_OPS_INTR).
 *              RX poll: DebugFramework_WaitChar() (UART_OPS_POLL).
 *
 * @note        Call DebugFramework_Init() after PRV_UART_Init().
 * @note        user_uart_isr.c → DebugFramework_IrqHandler().
 *
 * Copyright 2024 Tiremo. All rights reserved.
 ******************************************************************************/

#ifndef DEBUG_FRAMEWORK_H_
#define DEBUG_FRAMEWORK_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdbool.h>
#include <stdint.h>

#define TIREMO_DEBUG_RX_RING_SIZE       (128U)

/*-------------------------------------------------------------------------*//**
 * @brief         DebugFramework_RxCallback_t
 * @details       Invoked from UART0 IRQ for each received byte.
 *//*-------------------------------------------------------------------------*/
typedef void (*DebugFramework_RxCallback_t)(uint8_t byte);

/*-------------------------------------------------------------------------*//**
 * @brief         DebugFramework_Init
 * @param         None
 * @return        true on success, false on failure
 *//*-------------------------------------------------------------------------*/
bool DebugFramework_Init(void);

/*-------------------------------------------------------------------------*//**
 * @brief         DebugFramework_Uninit
 * @param         None
 * @return        None
 *//*-------------------------------------------------------------------------*/
void DebugFramework_Uninit(void);

/*-------------------------------------------------------------------------*//**
 * @brief         DebugFramework_SetRxCallback
 * @param[in]     pfn     RX callback (NULL to unregister)
 * @return        None
 *//*-------------------------------------------------------------------------*/
void DebugFramework_SetRxCallback(DebugFramework_RxCallback_t pfn);

/*-------------------------------------------------------------------------*//**
 * @brief         DebugFramework_IrqHandler
 * @param[in]     un32Event   HAL UART event flags
 * @param[in]     pContext    HAL context (unused)
 * @return        None
 *//*-------------------------------------------------------------------------*/
void DebugFramework_IrqHandler(uint32_t un32Event, void *pContext);

/*-------------------------------------------------------------------------*//**
 * @brief         DebugFramework_PutChar
 * @param[in]     ch      character to transmit
 * @return        None
 *//*-------------------------------------------------------------------------*/
void DebugFramework_PutChar(uint8_t ch);

/*-------------------------------------------------------------------------*//**
 * @brief         DebugFramework_Puts
 * @param[in]     str     null-terminated string
 * @return        None
 *//*-------------------------------------------------------------------------*/
void DebugFramework_Puts(const char *str);

/*-------------------------------------------------------------------------*//**
 * @brief         DebugFramework_PutsLine
 * @param[in]     str     null-terminated string (CRLF appended)
 * @return        None
 *//*-------------------------------------------------------------------------*/
void DebugFramework_PutsLine(const char *str);

/*-------------------------------------------------------------------------*//**
 * @brief         DebugFramework_PutDec
 * @param[in]     num     8-bit value (printed as 000–255)
 * @return        None
 *//*-------------------------------------------------------------------------*/
void DebugFramework_PutDec(uint8_t num);

/*-------------------------------------------------------------------------*//**
 * @brief         DebugFramework_PutDec16
 * @param[in]     num     16-bit unsigned decimal value
 * @return        None
 *//*-------------------------------------------------------------------------*/
void DebugFramework_PutDec16(uint16_t num);

/*-------------------------------------------------------------------------*//**
 * @brief         DebugFramework_PutDec32
 * @param[in]     num     32-bit unsigned decimal value
 * @return        None
 *//*-------------------------------------------------------------------------*/
void DebugFramework_PutDec32(uint32_t num);

/*-------------------------------------------------------------------------*//**
 * @brief         DebugFramework_PutHex
 * @param[in]     num     8-bit value (two hex digits)
 * @return        None
 *//*-------------------------------------------------------------------------*/
void DebugFramework_PutHex(uint8_t num);

/*-------------------------------------------------------------------------*//**
 * @brief         DebugFramework_PutHex16
 * @param[in]     num     16-bit value (four hex digits)
 * @return        None
 *//*-------------------------------------------------------------------------*/
void DebugFramework_PutHex16(uint16_t num);

/*-------------------------------------------------------------------------*//**
 * @brief         DebugFramework_PutHex32
 * @param[in]     num     32-bit value (eight hex digits)
 * @return        None
 *//*-------------------------------------------------------------------------*/
void DebugFramework_PutHex32(uint32_t num);

/*-------------------------------------------------------------------------*//**
 * @brief         DebugFramework_Printf
 * @param[in]     format  printf-style format string
 * @param         ...     format arguments
 * @return        None
 *//*-------------------------------------------------------------------------*/
void DebugFramework_Printf(const char *format, ...);

/*-------------------------------------------------------------------------*//**
 * @brief         DebugFramework_WaitChar
 * @details       Blocks until one byte is received (UART_OPS_POLL).
 * @return        received byte
 *//*-------------------------------------------------------------------------*/
uint8_t DebugFramework_WaitChar(void);

/*-------------------------------------------------------------------------*//**
 * @brief         DebugFramework_GetChar
 * @param         None
 * @return        received byte, or 0 if RX ring is empty
 *//*-------------------------------------------------------------------------*/
uint8_t DebugFramework_GetChar(void);

/*-------------------------------------------------------------------------*//**
 * @brief         DebugFramework_Read
 * @param[out]    pBuffer     destination buffer
 * @param[in]     maxLength   maximum bytes to read
 * @return        number of bytes read
 *//*-------------------------------------------------------------------------*/
uint32_t DebugFramework_Read(uint8_t *pBuffer, uint32_t maxLength);

/*-------------------------------------------------------------------------*//**
 * @brief         DebugFramework_IsDataAvailable
 * @param         None
 * @return        true if RX ring has unread data
 *//*-------------------------------------------------------------------------*/
bool DebugFramework_IsDataAvailable(void);

/*-------------------------------------------------------------------------*//**
 * @brief         DebugFramework_IsInitialized
 * @param         None
 * @return        true if DebugFramework_Init() succeeded
 *//*-------------------------------------------------------------------------*/
bool DebugFramework_IsInitialized(void);

#ifdef __cplusplus
}
#endif

#endif /* DEBUG_FRAMEWORK_H_ */
