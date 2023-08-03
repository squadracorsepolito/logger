/**
 * @file      debug.h
 * @prefix    DBG
 * @author    Simone Ruffini [simone.ruffini@squadracorsepolito.com | simone.ruffini.work@gmail.com]
 * @date      Thu Aug  3 03:37:49 PM CEST 2023
 * 
 * @brief     
 * 
 * @license Licensed under "THE BEER-WARE LICENSE", Revision 69 (see LICENSE)
 */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef _DBG_H_
#define _DBG_H_

/* Includes ------------------------------------------------------------------*/
#include "logger.h"

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
#define DBG_LOG_BUF_LEN 4096

/* Exported variables --------------------------------------------------------*/
extern LOGGER_HandleTypeDef logger_handle;

/* Exported macros -----------------------------------------------------------*/
#define VA_ARGS(...) , ##__VA_ARGS__
#define DBG_log(mode, template, ...) LOGGER_log(&logger_handle, mode, template VA_ARGS(__VA_ARGS__))

/* Exported functions --------------------------------------------------------*/
void DBG_init(void);
void DBG_routine(void);
/* Private types -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private constants ---------------------------------------------------------*/
/* Private Macros -----------------------------------------------------------*/
#endif
