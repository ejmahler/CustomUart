#include "SoftwareUart.h"

SoftwareUart uart;

void setup() {
  uart.begin(4800);
  Serial.begin(4800);
}

void loop() {
  // echo anything we get on our serial over our custom uart connection
  while(Serial.available()) {
    uart.write(Serial.read());
  }

  // echo anything we get from uart over our our serial connection
  while(uart.available()) {
    Serial.write(uart.read());
  }
}
