#include "stm32f1xx.h"

volatile uint8_t rx_byte;

void SystemClock_Config(void);
void GPIO_Init(void);
void UART1_Init(void);

int main(void) {
    SystemClock_Config();
    GPIO_Init();
    UART1_Init();

    while (1) {
        __WFI(); 
    }
}

void USART1_IRQHandler(void) {
    if (USART1->SR & USART_SR_RXNE) {
        rx_byte = USART1->DR;
        if (rx_byte == '\r' || rx_byte == '\n') return;

        char *prefix = "[RX]";
        while(*prefix) {
            while (!(USART1->SR & USART_SR_TXE));
            USART1->DR = *prefix++;
        }
        while (!(USART1->SR & USART_SR_TXE));
        USART1->DR = rx_byte;
        while (!(USART1->SR & USART_SR_TXE));
        USART1->DR = '\r';
        while (!(USART1->SR & USART_SR_TXE));
        USART1->DR = '\n';
    }
}

void GPIO_Init(void) {
    RCC->APB2ENR |= RCC_APB2ENR_IOPAEN | RCC_APB2ENR_USART1EN;
    GPIOA->CRH &= 0xFFFFF00F;
    GPIOA->CRH |= 0x000004B0;
}

void UART1_Init(void) {
    USART1->BRR = 0x1D4C;
    USART1->CR1 |= USART_CR1_UE | USART_CR1_TE | USART_CR1_RE | USART_CR1_RXNEIE;
    NVIC_EnableIRQ(USART1_IRQn);
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