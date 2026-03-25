#include "stm32f1xx.h"
#include <string.h>

uint8_t tx_data[50] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890!@#$%^&*()_+";

void Clock_Config_72MHz(void);
void GPIO_Init(void);
void UART1_Init(void);
void UART1_Transmit(uint8_t *data, uint16_t size);
uint8_t UART1_Receive(void);

int main(void) {
    Clock_Config_72MHz();
    GPIO_Init();
    UART1_Init();

    // 1. Send the Array (Requirement)
    UART1_Transmit(tx_data, 50);
    UART1_Transmit((uint8_t*)"\r\n[Polling Mode] Type to Echo:\r\n", 30);

    while (1) {
        uint8_t received = UART1_Receive(); // Blocking wait
        UART1_Transmit(&received, 1);       // Echo back
    }
}

// --- Driver Functions ---
void Clock_Config_72MHz(void) {
    RCC->CR |= RCC_CR_HSEON;
    while (!(RCC->CR & RCC_CR_HSERDY));
    FLASH->ACR |= FLASH_ACR_LATENCY_2;
    RCC->CFGR |= (RCC_CFGR_PLLSRC | RCC_CFGR_PLLMULL9 | RCC_CFGR_PPRE1_DIV2);
    RCC->CR |= RCC_CR_PLLON;
    while (!(RCC->CR & RCC_CR_PLLRDY));
    RCC->CFGR |= RCC_CFGR_SW_PLL;
    while ((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_PLL);
}
void GPIO_Init(void) {
    RCC->APB2ENR |= RCC_APB2ENR_IOPAEN | RCC_APB2ENR_USART1EN;
    GPIOA->CRH &= ~(0xFF0); GPIOA->CRH |= 0x4B0; // PA9=Alt-PP, PA10=Input
}
void UART1_Init(void) {
    USART1->BRR = 0x1D4C; // 9600 @ 72MHz
    USART1->CR1 |= (USART_CR1_UE | USART_CR1_TE | USART_CR1_RE);
}
void UART1_Transmit(uint8_t *data, uint16_t size) {
    for (uint16_t i = 0; i < size; i++) {
        while (!(USART1->SR & USART_SR_TXE));
        USART1->DR = data[i];
    }
}
uint8_t UART1_Receive(void) {
    while (!(USART1->SR & USART_SR_RXNE));
    return (uint8_t)(USART1->DR & 0xFF);
}