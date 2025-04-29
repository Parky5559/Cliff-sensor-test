#include <stdint.h>
#include <stdbool.h>
#include "driverlib/interrupt.h"
#include "tm4c123gh6pm.h"
#include "uart-interrupt.h"

volatile char received_char;
volatile int scan_active = 0;

void uart_interrupt_init(void) {
    SYSCTL_RCGCUART_R |= SYSCTL_RCGCUART_R1;
    SYSCTL_RCGCGPIO_R |= SYSCTL_RCGCGPIO_R1;

    while ((SYSCTL_PRGPIO_R & SYSCTL_PRGPIO_R1) == 0) {};
    while ((SYSCTL_PRUART_R & SYSCTL_PRUART_R1) == 0) {};

    GPIO_PORTB_AFSEL_R |= 0x03;
    GPIO_PORTB_PCTL_R |= (GPIO_PCTL_PB0_U1RX | GPIO_PCTL_PB1_U1TX);
    GPIO_PORTB_DEN_R |= 0x03;

    UART1_CTL_R &= ~UART_CTL_UARTEN;
    UART1_IBRD_R = 8;
    UART1_FBRD_R = 44;

    UART1_LCRH_R = UART_LCRH_WLEN_8;
    UART1_CC_R = 0x0;
    UART1_IM_R |= UART_IM_RXIM;
    NVIC_PRI1_R = (NVIC_PRI1_R & 0xFF0FFFFF) | 0x00200000;
    NVIC_EN0_R |= (1 << 6);

    IntRegister(INT_UART1, UART1_Handler);
    IntMasterEnable();

    UART1_CTL_R |= (UART_CTL_UARTEN | UART_CTL_RXE | UART_CTL_TXE);
}

void UART1_Handler(void) {
    received_char = UART1_DR_R & 0xFF;
    uart_sendChar(received_char);

    if (received_char == 'g') {
        scan_active = 1;
        uart_sendStr("\r\nScan Resumed\r\n");
    }
    else if (received_char == 's') {
        scan_active = 0;
        uart_sendStr("\r\nScan Stopped\r\n");
    }

    UART1_ICR_R |= UART_ICR_RXIC;
}


void uart_sendChar(char data) {
    while ((UART1_FR_R & UART_FR_TXFF));
    UART1_DR_R = data;
}

void uart_sendStr(const char *data) {
    while (*data) {
        uart_sendChar(*data++);
    }
}
