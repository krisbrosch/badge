#ifndef UART_H
#define UART_H

#include <stdlib.h>

#define RX_BUFFER_LEN 512
#define TX_BUFFER_LEN 512
#define SER_MODULE EUSCI_A0_BASE
#define UART2_MODULE EUSCI_A2_BASE

// Print and printf implementations for UARTs
void uart_printf(uint32_t module, const char *format, ...);
void uart_print(uint32_t module, const char *s);

// Wrappers for uart_print/uart_printf using the SER_MODULE for convenience
void ser_printf(const char *format, ...);
void ser_print(const char *s);
int ser_getLine(char *s, size_t length);
int32_t ser_peek();

void uart_txSync(uint32_t module, const uint8_t *s, size_t length);
void uart_tx(uint32_t module, const uint8_t *s, size_t length);
void uart_txByteSync(uint32_t module, uint8_t byte);
int uart_rx(uint32_t module, uint8_t *s, size_t length);
int32_t uart_rxByte(uint32_t module);
uint8_t uart_rxByteWait(uint32_t module);

// Gets a line of text from the uart. Won't return until the line is read. Ignores \r, but is terminated by \n. Handles backspace.
// Returns the null-terminated line (without \n) in s. If echo is true, echos input back.
int uart_getLine(uint32_t module, char *s, size_t length, int echo);

// Asynchronously checks if data is available for reading. If so, returns the
// first byte of that data without removing it from the buffer; 
// if not, returns -1.
int32_t uart_peek(uint32_t module);

// Initializes the following UARTs:
// - eUSCI_A0 as UART at 115200 using p1.2 as RX and p1.3 as TX
// - eUSCI_A2 as UART at 115200 using p3.2 as RX and p3.3 as TX
void uart_init();
// Disable the above UARTs, leaving them as GPIO input/output for RX/TX pins
void uart_disable();

#endif
