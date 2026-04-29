#pragma once

constexpr uint8_t TX_PIN = 9;
inline __attribute__((always_inline)) bool readRx() { return PINB & (1 << 0) > 0; }
inline __attribute__((always_inline)) void writeTx(uint8_t value) { 
  const uint8_t mask = 1 << 1;
  if(value) {
    PORTB |= mask;
  } else {
    PORTB &= ~mask;
  }
}