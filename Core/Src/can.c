/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    can.c
  * @brief   This file provides code for the configuration
  *          of the CAN instances.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "can.h"


/* USER CODE BEGIN 0 */
#include "debug.h"
#include "rtc.h"

/* USER CODE END 0 */

CAN_HandleTypeDef hcan1;

/* CAN1 init function */
void MX_CAN1_Init(void) {
    /* USER CODE BEGIN CAN1_Init 0 */

    /* USER CODE END CAN1_Init 0 */

    /* USER CODE BEGIN CAN1_Init 1 */

    /* USER CODE END CAN1_Init 1 */
    hcan1.Instance                  = CAN1;
    hcan1.Init.Prescaler            = 16;
    hcan1.Init.Mode                 = CAN_MODE_NORMAL;
    hcan1.Init.SyncJumpWidth        = CAN_SJW_1TQ;
    hcan1.Init.TimeSeg1             = CAN_BS1_1TQ;
    hcan1.Init.TimeSeg2             = CAN_BS2_1TQ;
    hcan1.Init.TimeTriggeredMode    = DISABLE;
    hcan1.Init.AutoBusOff           = ENABLE;
    hcan1.Init.AutoWakeUp           = DISABLE;
    hcan1.Init.AutoRetransmission   = DISABLE;
    hcan1.Init.ReceiveFifoLocked    = DISABLE;
    hcan1.Init.TransmitFifoPriority = DISABLE;
    if (HAL_CAN_Init(&hcan1) != HAL_OK) {
        Error_Handler();
    }
    /* USER CODE BEGIN CAN1_Init 2 */

    CAN_FilterTypeDef filter;
    filter.FilterMode = CAN_FILTERMODE_IDMASK;

    filter.FilterMaskIdLow  = 0x0U;  // low side of the Interval: Mask nothing
    filter.FilterMaskIdHigh = 0x0U;  // high side of the Interval: Mask nothing

    /* HAL considers IdLow and IdHigh not as just the ID of the can message but
        as the combination of: 
        STDID + RTR + IDE + 4 most significant bits of EXTID
    */
    filter.FilterIdLow          = 0x0U;              // meaningless since mask is 0
    filter.FilterIdHigh         = 0x0U;              // meaningless since mask is 0
    filter.FilterFIFOAssignment = CAN_FILTER_FIFO0;  // save messages in CAN fifo0
    filter.FilterBank           = 0;
    filter.FilterScale          = CAN_FILTERSCALE_32BIT;
    filter.FilterActivation     = ENABLE;

    HAL_CAN_ConfigFilter(&hcan1, &filter);

    uint32_t can_int_err_notifications = CAN_IT_ERROR_WARNING | CAN_IT_ERROR_PASSIVE | CAN_IT_BUSOFF |
                                         CAN_IT_LAST_ERROR_CODE | CAN_IT_ERROR;

    HAL_CAN_ActivateNotification(&hcan1, can_int_err_notifications | CAN_IT_RX_FIFO0_MSG_PENDING);

    /* USER CODE END CAN1_Init 2 */
}

void HAL_CAN_MspInit(CAN_HandleTypeDef *canHandle) {
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    if (canHandle->Instance == CAN1) {
        /* USER CODE BEGIN CAN1_MspInit 0 */

        /* USER CODE END CAN1_MspInit 0 */
        /* CAN1 clock enable */
        __HAL_RCC_CAN1_CLK_ENABLE();

        __HAL_RCC_GPIOA_CLK_ENABLE();
        /**CAN1 GPIO Configuration
    PA11     ------> CAN1_RX
    PA12     ------> CAN1_TX
    */
        GPIO_InitStruct.Pin       = GPIO_PIN_11 | GPIO_PIN_12;
        GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull      = GPIO_NOPULL;
        GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_VERY_HIGH;
        GPIO_InitStruct.Alternate = GPIO_AF9_CAN1;
        HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

        /* CAN1 interrupt Init */
        HAL_NVIC_SetPriority(CAN1_RX0_IRQn, 0, 0);
        HAL_NVIC_EnableIRQ(CAN1_RX0_IRQn);
        HAL_NVIC_SetPriority(CAN1_RX1_IRQn, 0, 0);
        HAL_NVIC_EnableIRQ(CAN1_RX1_IRQn);
        /* USER CODE BEGIN CAN1_MspInit 1 */

        /* USER CODE END CAN1_MspInit 1 */
    }
}

void HAL_CAN_MspDeInit(CAN_HandleTypeDef *canHandle) {
    if (canHandle->Instance == CAN1) {
        /* USER CODE BEGIN CAN1_MspDeInit 0 */

        /* USER CODE END CAN1_MspDeInit 0 */
        /* Peripheral clock disable */
        __HAL_RCC_CAN1_CLK_DISABLE();

        /**CAN1 GPIO Configuration
    PA11     ------> CAN1_RX
    PA12     ------> CAN1_TX
    */
        HAL_GPIO_DeInit(GPIOA, GPIO_PIN_11 | GPIO_PIN_12);

        /* CAN1 interrupt Deinit */
        HAL_NVIC_DisableIRQ(CAN1_RX0_IRQn);
        HAL_NVIC_DisableIRQ(CAN1_RX1_IRQn);
        /* USER CODE BEGIN CAN1_MspDeInit 1 */

        /* USER CODE END CAN1_MspDeInit 1 */
    }
}

/* USER CODE BEGIN 1 */
static void _CAN_error_handler(CAN_HandleTypeDef *hcan);

static void _CAN_error_handler(CAN_HandleTypeDef *hcan) {
    uint32_t err_code = HAL_CAN_GetError(hcan);

    if ((err_code & HAL_CAN_ERROR_EWG) == HAL_CAN_ERROR_EWG)
        DBG_log(LOGGER_ERROR, "Protocol error warning");
    if ((err_code & HAL_CAN_ERROR_EPV) == HAL_CAN_ERROR_EPV)
        DBG_log(LOGGER_ERROR, "Error passive");
    if ((err_code & HAL_CAN_ERROR_BOF) == HAL_CAN_ERROR_BOF)
        DBG_log(LOGGER_ERROR, "Bus-off error");
    if ((err_code & HAL_CAN_ERROR_STF) == HAL_CAN_ERROR_STF)
        DBG_log(LOGGER_ERROR, "Stuff error");
    if ((err_code & HAL_CAN_ERROR_FOR) == HAL_CAN_ERROR_FOR)
        DBG_log(LOGGER_ERROR, "Form error");
    if ((err_code & HAL_CAN_ERROR_ACK) == HAL_CAN_ERROR_ACK)
        DBG_log(LOGGER_ERROR, "ACK error");
    if ((err_code & HAL_CAN_ERROR_BR) == HAL_CAN_ERROR_BR)
        DBG_log(LOGGER_ERROR, "Bit Recessive error");
    if ((err_code & HAL_CAN_ERROR_BD) == HAL_CAN_ERROR_BD)
        DBG_log(LOGGER_ERROR, "Bit Dominant error");
    if ((err_code & HAL_CAN_ERROR_CRC) == HAL_CAN_ERROR_CRC)
        DBG_log(LOGGER_ERROR, "CRC error");
    if ((err_code & HAL_CAN_ERROR_RX_FOV0) == HAL_CAN_ERROR_RX_FOV0)
        DBG_log(LOGGER_ERROR, "FIFO 0 overrun error");
    if ((err_code & HAL_CAN_ERROR_RX_FOV1) == HAL_CAN_ERROR_RX_FOV1)
        DBG_log(LOGGER_ERROR, "FIFO 1 overrun error");
    if ((err_code & HAL_CAN_ERROR_TX_ALST0) == HAL_CAN_ERROR_TX_ALST0)
        DBG_log(LOGGER_ERROR, "TX 0 arbitration lost error");
    if ((err_code & HAL_CAN_ERROR_TX_TERR0) == HAL_CAN_ERROR_TX_TERR0)
        DBG_log(LOGGER_ERROR, "TX 0 transmit error");
    if ((err_code & HAL_CAN_ERROR_TX_ALST1) == HAL_CAN_ERROR_TX_ALST1)
        DBG_log(LOGGER_ERROR, "TX 1 arbitration lost error");
    if ((err_code & HAL_CAN_ERROR_TX_TERR1) == HAL_CAN_ERROR_TX_TERR1)
        DBG_log(LOGGER_ERROR, "TX 1 transmit error");
    if ((err_code & HAL_CAN_ERROR_TX_ALST2) == HAL_CAN_ERROR_TX_ALST2)
        DBG_log(LOGGER_ERROR, "TX 2 arbitration lost error");
    if ((err_code & HAL_CAN_ERROR_TX_TERR2) == HAL_CAN_ERROR_TX_TERR2)
        DBG_log(LOGGER_ERROR, "TX 2 transmit error");
    if ((err_code & HAL_CAN_ERROR_TIMEOUT) == HAL_CAN_ERROR_TIMEOUT)
        DBG_log(LOGGER_ERROR, "Timeout error");
    if ((err_code & HAL_CAN_ERROR_NOT_INITIALIZED) == HAL_CAN_ERROR_NOT_INITIALIZED)
        DBG_log(LOGGER_ERROR, "CAN bus not initialized");
    if ((err_code & HAL_CAN_ERROR_NOT_READY) == HAL_CAN_ERROR_NOT_READY)
        DBG_log(LOGGER_ERROR, "CAN bus not ready");
    if ((err_code & HAL_CAN_ERROR_NOT_STARTED) == HAL_CAN_ERROR_NOT_STARTED)
        DBG_log(LOGGER_ERROR, "CAN bus not started");
    if ((err_code & HAL_CAN_ERROR_PARAM) == HAL_CAN_ERROR_PARAM)
        DBG_log(LOGGER_ERROR, "Parameter error");
    if ((err_code & HAL_CAN_ERROR_INTERNAL) == HAL_CAN_ERROR_INTERNAL)
        DBG_log(LOGGER_ERROR, "Internal error");

    uint16_t rec_val = (uint16_t)((hcan->Instance->ESR && CAN_ESR_REC_Msk) >> CAN_ESR_REC_Pos);
    if (rec_val > 0) {
        DBG_log(LOGGER_ERROR, "REC (Receive Error Counter) %d", rec_val);
    }

    uint16_t tec_val = (uint16_t)((hcan->Instance->ESR && CAN_ESR_TEC_Msk) >> CAN_ESR_TEC_Pos);
    if (tec_val > 0) {
        DBG_log(LOGGER_ERROR, "TEC (Transmit Error Counter) %d", tec_val);
    }
}

HAL_StatusTypeDef CAN_wait(CAN_HandleTypeDef *hcan, uint8_t timeout) {
    uint32_t tick = HAL_GetTick();
    while (HAL_CAN_GetTxMailboxesFreeLevel(hcan) == 0) {
        if (HAL_GetTick() - tick > timeout)
            return HAL_TIMEOUT;
    }
    return HAL_OK;
}

HAL_StatusTypeDef CAN_send(CAN_HandleTypeDef *hcan, uint8_t *buffer, CAN_TxHeaderTypeDef *header) {
    if (CAN_wait(hcan, 1) != HAL_OK)
        return HAL_TIMEOUT;
    uint32_t mailbox;

    volatile HAL_StatusTypeDef status = HAL_CAN_AddTxMessage(hcan, header, buffer, &mailbox);

    return status;
}

void HAL_CAN_ErrorCallback(CAN_HandleTypeDef *hcan) {
    _CAN_error_handler(hcan);
}

void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan) {
    static CAN_RxHeaderTypeDef rx_header;
    static uint8_t buffer[8] = {0};
    static RTC_TimeTypeDef rtc_time;
    static RTC_DateTypeDef rtc_date;

    if (HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO0, &rx_header, buffer) != HAL_OK) {
        return;
    }

    /**
     if(rx_header.StdId == MCB_SCANNER_ && rx_header.DLC == 1U ){
         if(buffer[0] == 0xFFU && buffer[1] == 0x00U)
            NVIC_SystemReset();
     }
     */

    // Always call GetDate after GetTime otherwise time shwadow register will not be updated
    HAL_RTC_GetTime(&hrtc, &rtc_time, RTC_FORMAT_BCD);
    HAL_RTC_GetDate(&hrtc, &rtc_date, RTC_FORMAT_BCD);

    // Format can data

    // Add can data to circular buffer
}

/* USER CODE END 1 */
