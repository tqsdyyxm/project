/**
  ******************************************************************************
  * @file    bsp_uart.c
  * @brief   USART1 中断收发 + 环形缓冲实现
  *
  * 设计说明：
  *   - 接收：USART1 中断逐字节接收，存入 256 字节环形缓冲；
  *   - 发送：数据先写入 512 字节发送环形缓冲，中断自动搬出，
  *           调用方永远不会被阻塞。
  ******************************************************************************
  */

#include "bsp_uart.h"
#include <string.h>

#define RX_BUF_SIZE 256U
#define TX_BUF_SIZE 512U

static uint8_t  s_rx_buf[RX_BUF_SIZE];
static volatile uint16_t s_rx_head = 0;
static volatile uint16_t s_rx_tail = 0;
/* 单字节接收的"落地缓冲"：必须独立于环形缓冲！
 * 若直接收到 s_rx_buf[0]，会覆盖环形缓冲第 0 号槽位，
 * 导致每行数据的第一个字符被后一个字节顶掉（指令永远解析失败）。 */
static uint8_t  s_rx_byte;

static uint8_t  s_tx_buf[TX_BUF_SIZE];
static volatile uint16_t s_tx_head = 0;
static volatile uint16_t s_tx_tail = 0;
static volatile uint8_t  s_tx_busy = 0;
static uint8_t  s_tx_byte;
static volatile uint8_t  s_uart_ready = 0;

static void uart_start_tx(void)
{
    if (s_tx_head != s_tx_tail)
    {
        s_tx_byte = s_tx_buf[s_tx_tail];
        s_tx_tail = (uint16_t)((s_tx_tail + 1U) % TX_BUF_SIZE);
        HAL_UART_Transmit_IT(&huart_synex, &s_tx_byte, 1);
    }
    else
    {
        s_tx_busy = 0;
    }
}

void BSP_Uart_Init(void)
{
    GPIO_InitTypeDef gpio = {0};

    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_USART1_CLK_ENABLE();

    /* PA9 -> USART1_TX（AF7） */
    gpio.Pin       = GPIO_PIN_9;
    gpio.Mode      = GPIO_MODE_AF_PP;
    gpio.Pull      = GPIO_NOPULL;
    gpio.Speed     = GPIO_SPEED_FREQ_HIGH;
    gpio.Alternate = GPIO_AF7_USART1;
    HAL_GPIO_Init(GPIOA, &gpio);

    /* PB7 -> USART1_RX（AF7），上拉避免悬空乱码 */
    gpio.Pin       = GPIO_PIN_7;
    gpio.Mode      = GPIO_MODE_AF_PP;
    gpio.Pull      = GPIO_PULLUP;
    gpio.Speed     = GPIO_SPEED_FREQ_HIGH;
    gpio.Alternate = GPIO_AF7_USART1;
    HAL_GPIO_Init(GPIOB, &gpio);

    huart_synex.Instance          = USART1;
    huart_synex.Init.BaudRate     = UART_BAUDRATE;
    huart_synex.Init.WordLength   = UART_WORDLENGTH_8B;
    huart_synex.Init.StopBits     = UART_STOPBITS_1;
    huart_synex.Init.Parity       = UART_PARITY_NONE;
    huart_synex.Init.Mode         = UART_MODE_TX_RX;
    huart_synex.Init.HwFlowCtl    = UART_HWCONTROL_NONE;
    huart_synex.Init.OverSampling = UART_OVERSAMPLING_16;
    if (HAL_UART_Init(&huart_synex) != HAL_OK)
    {
        Error_Handler();
    }

    /* 中断优先级 5：可在 ISR 内调用 FreeRTOS FromISR API */
    HAL_NVIC_SetPriority(USART1_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(USART1_IRQn);

    s_uart_ready = 1;
    /* 武装单字节接收（回调里每收到 1 字节就重新武装） */
    HAL_UART_Receive_IT(&huart_synex, &s_rx_byte, 1);
}

void BSP_Uart_Send(const uint8_t *data, uint32_t len)
{
    uint16_t next;

    if (s_uart_ready == 0 || data == 0 || len == 0)
    {
        return;
    }

    for (uint32_t i = 0; i < len; i++)
    {
        next = (uint16_t)((s_tx_head + 1U) % TX_BUF_SIZE);
        if (next == s_tx_tail)
        {
            break;              /* 发送缓冲满，丢弃剩余（调试数据可容忍丢帧） */
        }
        s_tx_buf[s_tx_head] = data[i];
        s_tx_head = next;
    }

    if (s_tx_busy == 0)
    {
        s_tx_busy = 1;
        uart_start_tx();
    }
}

void BSP_Uart_SendString(const char *str)
{
    if (str != 0)
    {
        BSP_Uart_Send((const uint8_t *)str, (uint32_t)strlen(str));
    }
}

uint32_t BSP_Uart_RxAvailable(void)
{
    return (uint32_t)((s_rx_head - s_rx_tail + RX_BUF_SIZE) % RX_BUF_SIZE);
}

int32_t BSP_Uart_ReadByte(void)
{
    uint8_t byte;

    if (s_rx_head == s_rx_tail)
    {
        return -1;
    }
    byte = s_rx_buf[s_rx_tail];
    s_rx_tail = (uint16_t)((s_rx_tail + 1U) % RX_BUF_SIZE);
    return (int32_t)byte;
}

/* ===================== HAL 回调（在中断中执行） ===================== */

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART1)
    {
        uint16_t next;

        /* 把落地缓冲中的字节存入环形缓冲（两者是不同存储） */
        next = (uint16_t)((s_rx_head + 1U) % RX_BUF_SIZE);
        if (next != s_rx_tail)
        {
            s_rx_buf[s_rx_head] = s_rx_byte;
            s_rx_head = next;
        }
        /* 重新武装下一次接收 */
        HAL_UART_Receive_IT(&huart_synex, &s_rx_byte, 1);
    }
}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART1)
    {
        uart_start_tx();
    }
}
