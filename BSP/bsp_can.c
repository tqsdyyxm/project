/**
  ******************************************************************************
  * @file    bsp_can.c
  * @brief   CAN1 驱动实现：1 Mbps、标准帧、中断接收
  *
  * 位时序（CAN 时钟 = PCLK1 = 42 MHz）：
  *   注意：HAL 的 Prescaler 就是分频系数本身（HAL 写寄存器时内部自动减 1，
 *   见 stm32f4xx_hal_can.c 中 BTR 写入 (Prescaler - 1U)）。
 *   42 MHz / 3 = 14 MHz tq；位时间 = 1 + BS1(11) + BS2(2) = 14 tq
 *   -> 恰好 1 Mbps，采样点 (1+11)/14 = 85.7%。
  ******************************************************************************
  */

#include "bsp_can.h"

volatile Can_Feedback_t g_feedback       = {0};
volatile uint32_t       g_feedback_tick  = 0;
volatile uint32_t       g_can_error_count = 0;

void BSP_Can_Init(void)
{
    GPIO_InitTypeDef     gpio = {0};
    CAN_FilterTypeDef    filter = {0};

    __HAL_RCC_GPIOD_CLK_ENABLE();
    __HAL_RCC_CAN1_CLK_ENABLE();

    /* PD0 -> CAN1_RX（AF9），上拉保证总线空闲电平 */
    gpio.Pin       = GPIO_PIN_0;
    gpio.Mode      = GPIO_MODE_AF_PP;
    gpio.Pull      = GPIO_PULLUP;
    gpio.Speed     = GPIO_SPEED_FREQ_HIGH;
    gpio.Alternate = GPIO_AF9_CAN1;
    HAL_GPIO_Init(GPIOD, &gpio);

    /* PD1 -> CAN1_TX（AF9） */
    gpio.Pin       = GPIO_PIN_1;
    gpio.Mode      = GPIO_MODE_AF_PP;
    gpio.Pull      = GPIO_NOPULL;
    gpio.Speed     = GPIO_SPEED_FREQ_HIGH;
    gpio.Alternate = GPIO_AF9_CAN1;
    HAL_GPIO_Init(GPIOD, &gpio);

    /* ---- CAN1：1 Mbps 标准帧 ---- */
    hcan_motor.Instance                  = CAN1;
    hcan_motor.Init.Prescaler            = 3;              /* 42 MHz / 3 = 14 MHz tq */
    hcan_motor.Init.Mode                 = CAN_MODE_NORMAL;
    hcan_motor.Init.SyncJumpWidth        = CAN_SJW_1TQ;
    hcan_motor.Init.TimeSeg1             = CAN_BS1_11TQ;
    hcan_motor.Init.TimeSeg2             = CAN_BS2_2TQ;
    hcan_motor.Init.TimeTriggeredMode    = DISABLE;
    hcan_motor.Init.AutoBusOff           = ENABLE;         /* 总线错误自动恢复 */
    hcan_motor.Init.AutoWakeUp           = DISABLE;
    hcan_motor.Init.AutoRetransmission   = DISABLE;
    hcan_motor.Init.ReceiveFifoLocked    = DISABLE;
    hcan_motor.Init.TransmitFifoPriority = DISABLE;
    if (HAL_CAN_Init(&hcan_motor) != HAL_OK)
    {
        Error_Handler();
    }

    /* ---- 滤波器：接收全部标准帧（电机 ID 未确认前最稳妥），
            回调里按 StdId 过滤。若要只收本电机，参考注释：
        filter.FilterIdHigh = (uint16_t)(CAN_FEEDBACK_STDID << 5);
        filter.FilterIdLow  = 0;
        filter.FilterMaskIdHigh = 0xFFFF;
        filter.FilterMaskIdLow  = 0xFFF8;
    ---- */
    filter.FilterActivation    = ENABLE;
    filter.FilterBank          = 0;
    filter.FilterFIFOAssignment = CAN_RX_FIFO0;
    filter.FilterIdHigh        = 0x0000;
    filter.FilterIdLow         = 0x0000;
    filter.FilterMaskIdHigh    = 0x0000;
    filter.FilterMaskIdLow     = 0x0000;
    filter.FilterMode          = CAN_FILTERMODE_IDMASK;
    filter.FilterScale         = CAN_FILTERSCALE_32BIT;
    if (HAL_CAN_ConfigFilter(&hcan_motor, &filter) != HAL_OK)
    {
        Error_Handler();
    }

    /* ---- 中断：RX FIFO0 消息 + 错误通知 ---- */
    HAL_CAN_ActivateNotification(&hcan_motor,
                                 CAN_IT_RX_FIFO0_MSG_PENDING | CAN_IT_ERROR);
    HAL_NVIC_SetPriority(CAN1_RX0_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(CAN1_RX0_IRQn);
    HAL_NVIC_SetPriority(CAN1_SCE_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(CAN1_SCE_IRQn);

    /* ---- 启动 CAN（ActivateNotification 已使能 RX/错误中断） ---- */
    if (HAL_CAN_Start(&hcan_motor) != HAL_OK)
    {
        Error_Handler();
    }
}

void BSP_Can_SendCurrent(int16_t current_raw)
{
    CAN_TxHeaderTypeDef tx_header;
    uint8_t             data[8] = {0};
    uint32_t            mailbox = 0;
    uint32_t            offset;

    /* 截断到合法范围 */
    if (current_raw > CURRENT_RAW_MAX)
    {
        current_raw = CURRENT_RAW_MAX;
    }
    if (current_raw < -CURRENT_RAW_MAX)
    {
        current_raw = -CURRENT_RAW_MAX;
    }

    /* 本电机电流位于 DATA[2*(ID-1) : 2*(ID-1)+1]，高字节在前 */
    offset = (uint32_t)(MOTOR_ID - 1) * 2U;
    data[offset]     = (uint8_t)((uint16_t)current_raw >> 8);
    data[offset + 1] = (uint8_t)((uint16_t)current_raw & 0xFFU);

    tx_header.StdId = CAN_CTRL_STDID;
    tx_header.ExtId = 0;
    tx_header.IDE   = CAN_ID_STD;
    tx_header.RTR   = CAN_RTR_DATA;
    tx_header.DLC   = 8;
    tx_header.TransmitGlobalTime = DISABLE;

    if (HAL_CAN_AddTxMessage(&hcan_motor, &tx_header, data, &mailbox) != HAL_OK)
    {
        /* 发送失败：邮箱满或总线错误，累计计数供诊断 */
        g_can_error_count++;
    }
}

/* ===================== HAL 回调（在中断中执行） ===================== */

/**
  * @brief  RX FIFO0 收到完整报文
  */
void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan)
{
    CAN_RxHeaderTypeDef rx_header;
    uint8_t             data[8];

    if (HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO0, &rx_header, data) != HAL_OK)
    {
        return;
    }

    /* 只处理本电机 ID 的反馈帧（C620 反馈：0x200 + ID） */
    if (rx_header.StdId == CAN_FEEDBACK_STDID &&
        rx_header.IDE == CAN_ID_STD &&
        rx_header.DLC >= 7)
    {
        g_feedback.angle_raw   = (uint16_t)((data[0] << 8) | data[1]);
        g_feedback.speed_rpm   = (int16_t)((uint16_t)((data[2] << 8) | data[3]));
        g_feedback.current_raw = (int16_t)((uint16_t)((data[4] << 8) | data[5]));
        g_feedback.temp        = (int8_t)data[6];
        g_feedback_tick        = HAL_GetTick();
    }
    /* FMP0 中断为电平触发：FIFO 中还有消息时会自动再次进入中断，
       无需重新武装接收。 */
}

/**
  * @brief  CAN 错误（总线错误 / BusOff 等）
  */
void HAL_CAN_ErrorCallback(CAN_HandleTypeDef *hcan)
{
    (void)hcan;
    g_can_error_count++;
}
