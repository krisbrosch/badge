#include <stdio.h>
#include <stdint.h>
#include <stdarg.h>
#include "driverlib.h"
#include "ringbuffer.h"
#include "uart.h"

uint8_t rxBuf0Data[RX_BUFFER_LEN];
uint8_t rxBuf2Data[RX_BUFFER_LEN];
uint8_t txBuf0Data[TX_BUFFER_LEN];
uint8_t txBuf2Data[TX_BUFFER_LEN];
struct ringbuffer rxBuf0, rxBuf2;
struct ringbuffer txBuf0, txBuf2;

const eUSCI_UART_Config uartConfig =
{
  EUSCI_A_UART_CLOCKSOURCE_SMCLK,
  26,
  0,
  111,
  EUSCI_A_UART_NO_PARITY,
  EUSCI_A_UART_LSB_FIRST,
  EUSCI_A_UART_ONE_STOP_BIT,
  EUSCI_A_UART_MODE,
  EUSCI_A_UART_OVERSAMPLING_BAUDRATE_GENERATION
};


void uart_vprintf(uint32_t module, const char *format, va_list args) {
  char buf[512];

  vsnprintf(buf, 512, format, args);
  buf[1023] = 0;
  uart_print(module, buf);
}

void uart_printf(uint32_t module, const char *format, ...) {
  va_list args;
  va_start(args, format);
  uart_vprintf(module, format, args);
  va_end(args);
}

void ser_printf(const char *format, ...) {
  va_list args;
  va_start(args, format);
  uart_vprintf(SER_MODULE, format, args);
  va_end(args);
}

void ser_print(const char *s) {
  uart_print(SER_MODULE, s);
}

int ser_getLine(char *s, size_t length) {
  return uart_getLine(SER_MODULE, s, length, 1);
}

int32_t ser_peek() {
  return uart_peek(SER_MODULE);
}

size_t strlen(const char *s) {
  size_t i = 0;
  while(s[i] != 0) { i++; }
  return i;
}

void uart_print(uint32_t module, const char *s) {
  uart_tx(module, (uint8_t *)s, strlen(s));
}

struct ringbuffer *whichRxBuf(uint32_t module) {
  if(module == UART2_MODULE) {
    return &rxBuf2;
  }
  return &rxBuf0;
}

struct ringbuffer *whichTxBuf(uint32_t module) {
  if(module == UART2_MODULE) {
    return &txBuf2;
  }
  return &txBuf0;
}

void uart_txSync(uint32_t module, const uint8_t *s, size_t length) {
  size_t i;
  uint_fast8_t byte;

  for(i = 0; i < length; i++) {
    byte = s[i];
    uart_txByteSync(module, byte);
  }
}

void uart_txByteSync(uint32_t module, uint8_t byte) {
  struct ringbuffer *buf;
  buf = whichTxBuf(module);
  while(!rb_empty(buf));
  while( !(MAP_UART_getInterruptStatus(module, EUSCI_A_UART_TRANSMIT_INTERRUPT_FLAG)) );
  MAP_UART_transmitData(module, byte);
}

void uart_tx(uint32_t module, const uint8_t *s, size_t length) {
  size_t i;
  uint_fast8_t byte;
  struct ringbuffer *buf;
  buf = whichTxBuf(module);

  for(i = 0; i < length; i++) {
    if(rb_full(buf)) {
      MAP_UART_enableInterrupt(module, EUSCI_A_UART_TRANSMIT_INTERRUPT);
      while(rb_full(buf));
    }
    byte = s[i];
    rb_insert(buf, byte);
  }
  MAP_UART_enableInterrupt(module, EUSCI_A_UART_TRANSMIT_INTERRUPT);
}

int uart_rx(uint32_t module, uint8_t *s, size_t length) {
  uint32_t i = 0;
  struct ringbuffer *buf;
  buf = whichRxBuf(module);
  while((!rb_empty(buf)) && (i < length)) {
    s[i++] = rb_remove(buf);
  }
  return i;
}

int32_t uart_rxByte(uint32_t module) {
  struct ringbuffer *buf;
  buf = whichRxBuf(module);
  if(rb_empty(buf))
    return -1;
  return rb_remove(buf);
}

uint8_t uart_rxByteWait(uint32_t module) {
  struct ringbuffer *buf;
  buf = whichRxBuf(module);
  while(rb_empty(buf));
  return rb_remove(buf);
}

int uart_getLine(uint32_t module, char *s, size_t length, int echo) {
  uint32_t i = 0;
  char c;
  int done = 0;
  struct ringbuffer *buf;
  buf = whichRxBuf(module);
  while((!done) && (i < (length - 1))) {
    while(rb_empty(buf)); // Wait for data
    c = rb_remove(buf);
    if(c == '\r') {
      if(echo) {
        uart_txByteSync(module, c);
        uart_txByteSync(module, '\n');
      }
      done = 1;
    } else if((c == 0x08) || (c == 0x7f)) { // backspace
      if(i > 0) {
        if(echo) {
          //uart_txByteSync(module, c);
          uart_txByteSync(module, 0x08);
          uart_txByteSync(module, 0x20);
          uart_txByteSync(module, 0x08);
        }
        i--;
      }
    } else if(c == '\n') {
      // Ignore \n
    } else {
      s[i++] = c;
      if(echo) {
        uart_txByteSync(module, c);
      }
    }
  }
  s[i] = 0;
  return i;
} 

int32_t uart_peek(uint32_t module) {
  struct ringbuffer *buf;
  buf = whichRxBuf(module);
  return rb_peek(buf);
}

void uart_init() {
  rb_init(&rxBuf0, rxBuf0Data, RX_BUFFER_LEN);
  rb_init(&txBuf0, txBuf0Data, TX_BUFFER_LEN);
  MAP_GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_P1, GPIO_PIN2, GPIO_PRIMARY_MODULE_FUNCTION);
  MAP_GPIO_setAsPeripheralModuleFunctionOutputPin(GPIO_PORT_P1, GPIO_PIN3, GPIO_PRIMARY_MODULE_FUNCTION);
  MAP_UART_initModule(EUSCI_A0_BASE, &uartConfig);
  MAP_UART_enableModule(EUSCI_A0_BASE);
  MAP_UART_enableInterrupt(EUSCI_A0_BASE, EUSCI_A_UART_RECEIVE_INTERRUPT);
  MAP_Interrupt_enableInterrupt(INT_EUSCIA0);

  rb_init(&rxBuf2, rxBuf2Data, RX_BUFFER_LEN);
  rb_init(&txBuf2, txBuf2Data, TX_BUFFER_LEN);
  MAP_GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_P3, GPIO_PIN2, GPIO_PRIMARY_MODULE_FUNCTION);
  MAP_GPIO_setAsPeripheralModuleFunctionOutputPin(GPIO_PORT_P3, GPIO_PIN3, GPIO_PRIMARY_MODULE_FUNCTION);
  MAP_UART_initModule(EUSCI_A2_BASE, &uartConfig);
  MAP_UART_enableModule(EUSCI_A2_BASE);
  MAP_UART_enableInterrupt(EUSCI_A2_BASE, EUSCI_A_UART_RECEIVE_INTERRUPT);
  MAP_Interrupt_enableInterrupt(INT_EUSCIA2);
}


// TODO: When RX buffer fills this causes hang because we don't read the UART's
// RX buf, the interrupt flag is never cleared, and the the interrupt repeatedly
// fires without software having a chance to empty the RX buffer.
void euscia0_isr(void) {
  uint16_t status;
  uint32_t newindex;

  status = UCA0IE & UCA0IFG;

  if(status & UCRXIFG) { // Byte RX'd
    // Manual rb_insert the RX'd byte
    newindex = (rxBuf0.end + 1) & rxBuf0.mask;
    if(newindex != rxBuf0.start) {
      rxBuf0.buf[rxBuf0.end] = UCA0RXBUF;
      rxBuf0.end = newindex;
    }
  } else if(status & UCTXIFG) { // Ready to TX
    // If TX buffer not empty, manually rb_remove and TX next byte
    if(txBuf0.start != txBuf0.end) {
      newindex = (txBuf0.start + 1) & txBuf0.mask;
      UCA0TXBUF = txBuf0.buf[txBuf0.start];
      txBuf0.start = newindex;
    }
    // If buffer etmpy now, clear TX interrupt flag
    if(txBuf0.start == txBuf0.end) {
      UCA0IE &= ~UCTXIE;
    }
  }
}

void euscia2_isr(void) {
  uint16_t status;
  uint32_t newindex;

  status = UCA2IE & UCA2IFG;

  if(status & UCRXIFG) { // Byte RX'd
    // Manual rb_insert the RX'd byte
    newindex = (rxBuf2.end + 1) & rxBuf2.mask;
    if(newindex != rxBuf2.start) {
      rxBuf2.buf[rxBuf2.end] = UCA2RXBUF;
      rxBuf2.end = newindex;
    }
  } else if(status & UCTXIFG) { // Ready to TX
    // If TX buffer not empty, manually rb_remove and TX next byte
    if(txBuf2.start != txBuf2.end) {
      newindex = (txBuf2.start + 1) & txBuf2.mask;
      UCA2TXBUF = txBuf2.buf[txBuf2.start];
      txBuf2.start = newindex;
    }
    // If buffer etmpy now, clear TX interrupt flag
    if(txBuf2.start == txBuf2.end) {
      UCA2IE &= ~UCTXIE;
    }
  }
}
