#include <Arduino.h>
#include "uart.h"
#include "pins.h"

uint16_t cyclesPerBit = 0;
uint8_t bitsPerRxFrame = 10; // 1 start bit, 8 data bits, 1 stop bit

void uart::begin(uint32_t baud) {
  if(baud < 320) { baud = 320; }
  cyclesPerBit = F_CPU / baud;

  // hold our tx pin high by default
  pinMode(9, OUTPUT);
  writeTx(1);

  // No interrupts except the falling edge capture interrupt at first. We'll enable the OCR1A and B interrupts as we need them
  TIMSK1 = 1 << ICIE1;

  // Enable the clock in normal mode
  TCCR1A = 0;
  TCCR1B = 1;
}
