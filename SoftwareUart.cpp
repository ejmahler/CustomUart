#include <Arduino.h>
#include "SoftwareUart.h"
#include "uart_internal.h"

void SoftwareUart::begin(uint32_t baud) {
  UartInternal::begin(baud);
}
void SoftwareUart::end() {
  // nothing to do for now
}

size_t SoftwareUart::write(uint8_t byte) {
  UartInternal::txBlocking(&byte, 1);
  return 1;
}
size_t SoftwareUart::write(const uint8_t *buffer, size_t size) {
  UartInternal::txBlocking(buffer, size);
}
int SoftwareUart::availableForWrite() {
  return UartInternal::txMaxQueueLen() - UartInternal::txQueueLen();
}
void SoftwareUart::flush() {
  
}

int SoftwareUart::available() {
  return UartInternal::rxQueueLen();
}
int SoftwareUart::read() {
  uint8_t out;
  if(UartInternal::readNonBlocking(out)) {
    return out;
  } else {
    return -1;
  }
}
int SoftwareUart::peek() {
  uint8_t out;
  if(UartInternal::peekNonBlocking(out)) {
    return out;
  } else {
    return -1;
  }
}
