#include <Arduino.h>
#include "uart.h"
#include "uart_internal.h"
#include "pins.h"

uint16_t UartInternal::cyclesPerBit = 0;
uint8_t UartInternal::rxBitsPerFrame = 10; // 1 start bit, 8 data bits, 1 stop bit

void UartInternal::begin(uint32_t baud) {
  // Minimum of 245 baud because we are using a 16-bit timer and below 245, cyclesPerBit doesn't fit in a 16 bit integer
  if(baud < 245) { baud = 245; }
  UartInternal::cyclesPerBit = F_CPU / baud;

  // hold our tx pin high by default
  pinMode(9, OUTPUT);
  writeTx(1);

  // No interrupts except the falling edge capture interrupt at first. We'll enable the OCR1A and B interrupts as we need them
  TIMSK1 = 1 << ICIE1;

  // Enable the clock in normal mode
  TCCR1A = 0;
  TCCR1B = 1;
}
