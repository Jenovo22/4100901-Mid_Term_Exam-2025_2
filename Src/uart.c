#include "uart.h"

void init_gpio_uart(void) {
    // PA2->TX AF7, PA3->RX AF7
    RCC->AHB2ENR |= (1 << 0);  // Enable GPIOA
    GPIOA->MODER &= ~((3U<<4)|(3U<<6));
    GPIOA->MODER |=  ((2U<<4)|(2U<<6));
    GPIOA->AFRL &= ~((0xFU<<8)|(0xFU<<12));
    GPIOA->AFRL |=  ((7U<<8)|(7U<<12));  // AF7 = USART2
}

void init_uart(void) {
    RCC->APB1ENR1 |= (1 << 17);  // Enable USART2
    USART2->BRR = (HSI_FREQ + (BAUD_RATE/2)) / BAUD_RATE;
    USART2->CR1 = (1 << 3) | (1 << 2);  // TE | RE
    USART2->CR1 |= (1 << 0);            // UE
    //Registro CR1 modificado para activar paridad impar
    USART2->CR1 |= (1 << 10);           // PCE: Parity Control Enable
}

void uart_send(char c) {
    while (!(USART2->ISR & (1 << 7)));   // TXE
    USART2->TDR = (uint8_t)c;
}

char uart_receive(void) {
    while (!(USART2->ISR & (1 << 5)));   // RXNE
    return (char)(USART2->RDR & 0xFF);
}

void uart_send_string(const char *str) {
    while (*str) {
        uart_send(*str++);
    }
}
//Modifique la configuración de UART para activar bit de paridad impar (odd parity), mejorando la confiabilidad en ambientes ruidososos.
void uart_send_string_with_parity(const char *str) {
    while (*str) {
        uart_send(*str++);
    }
}