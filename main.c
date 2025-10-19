#include "stm32f10x.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define LED_PIN  GPIO_Pin_0
#define LED_PORT GPIOA

typedef struct {
    uint16_t frequency;   // Hz
    uint16_t duty_cycle;  // %
} Signal_t;

QueueHandle_t xSignalQueue;

void GPIO_Config(void);
void USART1_Config(void);
void LED_Task(void *pvParameters);
void UART_Task(void *pvParameters);
int UART_ReadLine(char *buf, uint16_t maxlen);

int main(void)
{
    SystemInit();
    GPIO_Config();
    USART1_Config();

    xSignalQueue = xQueueCreate(5, sizeof(Signal_t));

    xTaskCreate(LED_Task,  "LED",  128, NULL, 2, NULL);
    xTaskCreate(UART_Task, "UART", 128, NULL, 1, NULL);

    vTaskStartScheduler();
    while (1);
}

void GPIO_Config(void)
{
    GPIO_InitTypeDef gpio;
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    gpio.GPIO_Pin   = LED_PIN;
    gpio.GPIO_Mode  = GPIO_Mode_Out_PP;
    gpio.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(LED_PORT, &gpio);
    GPIO_ResetBits(LED_PORT, LED_PIN);
}

void USART1_Config(void)
{
    GPIO_InitTypeDef gpio;
    USART_InitTypeDef usart;
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_USART1, ENABLE);

    // PA9 - TX
    gpio.GPIO_Pin   = GPIO_Pin_9;
    gpio.GPIO_Mode  = GPIO_Mode_AF_PP;
    gpio.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &gpio);

    // PA10 - RX
    gpio.GPIO_Pin   = GPIO_Pin_10;
    gpio.GPIO_Mode  = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &gpio);

    usart.USART_BaudRate   = 9600;
    usart.USART_WordLength = USART_WordLength_8b;
    usart.USART_StopBits   = USART_StopBits_1;
    usart.USART_Parity     = USART_Parity_No;
    usart.USART_Mode       = USART_Mode_Tx | USART_Mode_Rx;
    usart.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_Init(USART1, &usart);
    USART_Cmd(USART1, ENABLE);
}

//------------------------------------------
// Task 1: blink LED theo tin hieu nhan tu Queue
//------------------------------------------
void LED_Task(void *pvParameters)
{
    Signal_t signal;
    while (1)
    {
        if (xQueueReceive(xSignalQueue, &signal, portMAX_DELAY) == pdPASS)
        {
            TickType_t period   = pdMS_TO_TICKS(1000 / signal.frequency);
            TickType_t on_time  = (period * signal.duty_cycle) / 100;
            TickType_t off_time = period - on_time;
            TickType_t start    = xTaskGetTickCount();

            while ((xTaskGetTickCount() - start) < pdMS_TO_TICKS(5000))
            {
                GPIO_SetBits(LED_PORT, LED_PIN);
                vTaskDelay(on_time);
                GPIO_ResetBits(LED_PORT, LED_PIN);
                vTaskDelay(off_time);
            }
        }
    }
}

//------------------------------------------
// Task 2: doc chuoi UART và gui vào Queue
//------------------------------------------
void UART_Task(void *pvParameters)
{
    char buf[32];
    Signal_t signal;

    while (1)
    {
if (UART_ReadLine(buf, sizeof(buf)) > 0)
        {
            sscanf(buf, "%hu,%hu", &signal.frequency, &signal.duty_cycle);
            xQueueSend(xSignalQueue, &signal, 0);
        }
    }
}

//------------------------------------------
// Hàm doc mot dong tu UART (ket thuc bang \n)
//------------------------------------------
int UART_ReadLine(char *buf, uint16_t maxlen)
{
    uint16_t idx = 0;
    while (idx < maxlen - 1)
    {
        while (USART_GetFlagStatus(USART1, USART_FLAG_RXNE) == RESET)
        {
            vTaskDelay(pdMS_TO_TICKS(10)); 
        }
        char c = USART_ReceiveData(USART1);
        if (c == '\n' || c == '\r')
        {
            buf[idx] = '\0';
            return idx;
        }
        buf[idx++] = c;
    }
    buf[idx] = '\0';
    return idx;
}
