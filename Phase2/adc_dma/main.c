#include "stm32f1xx.h"
#include <stdio.h>

volatile uint16_t adc_buffer[32];
char msg[64];

void SystemClock_Config(void);
void GPIO_Init(void);
void DMA1_Init(void);
void ADC1_Init(void);
void UART1_Send(char* s);

int main(void) {
    SystemClock_Config();
    GPIO_Init();
    DMA1_Init();
    ADC1_Init();

    while (1) {
        uint16_t raw = adc_buffer[0];
        sprintf(msg, "V: %d.%02dV | NDTR: %ld\r\n", (raw*330)/4095/100, (raw*330)/4095%100, DMA1_Channel1->CNDTR);
        UART1_Send(msg);
        for (volatile int i = 0; i < 5000000; i++);
    }
}

void DMA1_Init(void) {
    RCC->AHBENR |= RCC_AHBENR_DMA1EN;
    DMA1_Channel1->CPAR = (uint32_t)&ADC1->DR;
    DMA1_Channel1->CMAR = (uint32_t)adc_buffer;
    DMA1_Channel1->CNDTR = 32;
    DMA1_Channel1->CCR = DMA_CCR_CIRC | DMA_CCR_MINC | DMA_CCR_MSIZE_0 | DMA_CCR_PSIZE_0 | DMA_CCR_EN;
}

void ADC1_Init(void) {
    RCC->APB2ENR |= RCC_APB2ENR_ADC1EN;
    RCC->CFGR |= RCC_CFGR_ADCPRE_DIV6;
    ADC1->CR2 |= ADC_CR2_CONT | ADC_CR2_DMA | ADC_CR2_ADON;
    ADC1->SQR3 = 0;
    for(int i=0; i<1000; i++);
    ADC1->CR2 |= ADC_CR2_CAL;
    while(ADC1->CR2 & ADC_CR2_CAL);
    ADC1->CR2 |= ADC_CR2_ADON;
}

void UART1_Send(char* s) {
    while (*s) {
        while (!(USART1->SR & USART_SR_TXE));
        USART1->DR = *s++;
    }
}

void GPIO_Init(void) {
    RCC->APB2ENR |= RCC_APB2ENR_IOPAEN | RCC_APB2ENR_USART1EN;
    GPIOA->CRL &= ~(0xF);
    GPIOA->CRH &= 0xFFFFF00F;
    GPIOA->CRH |= 0x000004B0;
    USART1->BRR = 0x1D4C;
    USART1->CR1 |= USART_CR1_UE | USART_CR1_TE;
}

void SystemClock_Config(void) {
    RCC->CR |= RCC_CR_HSEON;
    while (!(RCC->CR & RCC_CR_HSERDY));
    FLASH->ACR |= FLASH_ACR_LATENCY_2;
    RCC->CFGR |= (RCC_CFGR_PLLSRC | RCC_CFGR_PLLMULL9 | RCC_CFGR_PPRE1_DIV2);
    RCC->CR |= RCC_CR_PLLON;
    while (!(RCC->CR & RCC_CR_PLLRDY));
    RCC->CFGR |= RCC_CFGR_SW_PLL;
    while ((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_PLL);
}