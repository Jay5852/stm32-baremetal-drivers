#include "stm32f1xx.h"

uint8_t tx_data[50] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890!@#$%^&*()_+";
uint8_t rx_buffer[50];

void SystemClock_Config(void);
void GPIO_Init(void);
void DMA1_Init(void);
void UART1_DMA_Enable(void);

int main(void) {
    SystemClock_Config();
    GPIO_Init();
    DMA1_Init();
    UART1_DMA_Enable();
    while (1);
}

void DMA1_Init(void) {
    RCC->AHBENR |= RCC_AHBENR_DMA1EN;
    DMA1_Channel4->CPAR = (uint32_t)&USART1->DR;
    DMA1_Channel4->CMAR = (uint32_t)tx_data;
    DMA1_Channel4->CNDTR = 50;
    DMA1_Channel4->CCR |= DMA_CCR_DIR | DMA_CCR_MINC | DMA_CCR_EN;

    DMA1_Channel5->CPAR = (uint32_t)&USART1->DR;
    DMA1_Channel5->CMAR = (uint32_t)rx_buffer;
    DMA1_Channel5->CNDTR = 50;
    DMA1_Channel5->CCR |= DMA_CCR_MINC | DMA_CCR_EN;
}

void UART1_DMA_Enable(void) {
    USART1->BRR = 0x1D4C;
    USART1->CR3 |= USART_CR3_DMAT | USART_CR3_DMAR;
    USART1->CR1 |= USART_CR1_UE | USART_CR1_TE | USART_CR1_RE;
}

void GPIO_Init(void) {
    RCC->APB2ENR |= RCC_APB2ENR_IOPAEN | RCC_APB2ENR_USART1EN;
    GPIOA->CRH &= 0xFFFFF00F;
    GPIOA->CRH |= 0x000004B0;
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