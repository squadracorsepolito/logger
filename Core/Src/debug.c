/**
 * @file      debug.c
 * @prefix    DBG
 * @author    Simone Ruffini [simone.ruffini@squadracorsepolito.com | simone.ruffini.work@gmail.com]
 * @date      Thu Aug  3 03:37:49 PM CEST 2023
 * 
 * @brief     
 * 
 * @license Licensed under "THE BEER-WARE LICENSE", Revision 69 (see LICENSE)
 */

/* Includes ------------------------------------------------------------------*/
#include "debug.h"
#include "usart.h"
#include "logger.h"



/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/
char _DBG_log_buf[DBG_LOG_BUF_LEN];

/* Private function prototypes -----------------------------------------------*/

/* Exported variables --------------------------------------------------------*/
LOGGER_HandleTypeDef logger_handle;

/* Exported functions --------------------------------------------------------*/
STMLIBS_StatusTypeDef logger_flush(char *buffer, uint32_t size) {
    assert_param(buffer);

    if(DBG_HUART.gState != HAL_UART_STATE_READY)
        return STMLIBS_BUSY;

    HAL_UART_Transmit_IT(&DBG_HUART, (uint8_t*)buffer, size);
    return STMLIBS_OK;
}

void DBG_init(void) {
    LOGGER_init(&logger_handle, _DBG_log_buf, DBG_LOG_BUF_LEN, logger_flush);
}

void DBG_routine(void) {
    LOGGER_flush(&logger_handle);
}

/* Private functions ---------------------------------------------------------*/
