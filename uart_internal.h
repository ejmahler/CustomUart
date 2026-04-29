#pragma once

class UartInternal {
public:
  static bool isTxActive();
  static bool isRxActive();

  static uint16_t cyclesPerBit;
  static uint8_t rxBitsPerFrame;
};
