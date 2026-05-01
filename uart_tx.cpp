#include <Arduino.h>
#include "uart.h"
#include "uart_internal.h"
#include "pins.h"
#include "ring_buffer.h"

// ========================================================================================
// TRANSMIT (aka outgoing)
// ========================================================================================
volatile uint16_t txFrame = 0; // The current frame being transmitted. Contains more than 8 bits because we also put the stop bit(s) and parity bit (if configured) in here, and we also use a 1 on the end as a sentinel
RingBuffer<5> txQueue;

// A transmit is active if OCR1A interrupts are enabled
bool UartInternal::isTxActive() {
  return (TIMSK1 & (1 << OCIE1A)) != 0;
}

void beginTx() {
  // enable OCR1A interrupts
  const uint8_t interrupts = TIMSK1;
  TIMSK1 = interrupts | (1 << OCIE1A);

  // If an OCR1A interrupt is scheduled, cancel it
  TIFR1 |= 1 << OCF1A;
   
  // Schedule the first event for 200 cycles from now. That should give the tx code enough time to get out of the noInterrupt state
  OCR1A = TCNT1 + 200;
}
void endTx() {
  // shut off OCR1A interrupts
  const uint8_t interrupts = TIMSK1;
  TIMSK1 = interrupts & ~(1 << OCIE1A);
}

ISR(TIMER1_COMPA_vect) {
  uint16_t localFrame = txFrame;

  // Check the frame we're currently transmitting. We use a 1 as a sentinel value to indicate that the current frame is done, so if it's equal to 0 or 1, we have to start transmitting a new byte
  if(localFrame < 2) {
    // Check the queue. If there is a byte to transmit, we pop it, set the start bit on the tx line, and schedule the next interrupt
    if(!txQueue.isEmpty()) {
      writeTx(0);
      
      uint8_t nextByte;
      txQueue.popNonBlocking(nextByte);

      localFrame = static_cast<uint16_t>(nextByte) | 256 | 512; // append stop bit and the sentinel. todo: compute parity, add multiple stop bits, etc
    } else {
      // No more bytes to transfer, so stop the transmission
      endTx();
    }
  } else {
    // We have bits left to transmit in this frame, so transmit one of them
    writeTx(localFrame & 1);
    localFrame >>= 1;
  }
  txFrame = localFrame;
  OCR1A += UartInternal::cyclesPerBit;
}

size_t UartInternal::txNonBlocking(const uint8_t* buffer, size_t len) {
  noInterrupts();
  size_t numPushed = txQueue.pushNonBlocking(buffer, len);

  // If the transmit system is currently idle, start it
  if(!UartInternal::isTxActive()) {
    beginTx();
  }
  interrupts();
  return numPushed;
}
void UartInternal::txBlocking(const uint8_t* buffer, size_t len) {
  // Push as many bytes as we can at once
  size_t numPushed = txNonBlocking(buffer, len);

  // For whatever's left, push byte by byte. Note that we're assuming we can push bytes faster than they're depleted from the queue. If that assumption were to fail and the queue starved, it would shut off 
  for(size_t i = numPushed; i < len; i++) {
    txQueue.pushBlocking(buffer[i]);
  }
}

size_t UartInternal::txQueueLen() {
  noInterrupts();
  const uint8_t len = txQueue.len();
  interrupts();
  return len;
}
size_t UartInternal::txMaxQueueLen() {
  return txQueue.maxLen();
}