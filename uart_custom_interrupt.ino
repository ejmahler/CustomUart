#include "uart_internal.h"

void setup() {
  UartInternal::begin(4800);
  Serial.begin(4800);
}

void loop() {
  // echo anything we get on our serial over our custom uart connection
  while(Serial.available()) {
    UartInternal::tx(Serial.read());
  }

  // echo anything we get from uart over our our serial connection
  uint8_t rxByte;
  while(UartInternal::readByte_nonBlocking(rxByte)) {
    Serial.write(rxByte);
  }
}
