#include <Arduino.h>
#include "uart.h"
#include "pins.h"
#include "ring_buffer.h"

// ========================================================================================
// TRANSMIT (aka outgoing)
// ========================================================================================
extern uint16_t cyclesPerBit;
volatile uint16_t txFrame = 0; // The current frame being transmitted. Contains more than 8 bits because we also put the stop bit(s) and parity bit (if configured) in here, and we also use a 1 on the end as a sentinel
RingBuffer<5> txQueue;

// A transmit is active if OCR1A interrupts are enabled
bool isTxActive() {
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
      txQueue.pop(nextByte);

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
  OCR1A += cyclesPerBit;
}

size_t uart::tx_nonBlocking(uint8_t* buffer, size_t len) {
  noInterrupts();
  size_t numPushed = txQueue.pushGreedy(buffer, len);

  // If the transmit system is currently idle, start it
  if(!isTxActive()) {
    beginTx();
  }
  interrupts();
  return numPushed;
}
void uart::tx(uint8_t* buffer, size_t len) {
  size_t numPushed = tx_nonBlocking(buffer, len);

  // Block until everything is queued
  while(numPushed < len) {
    len -= numPushed;
    buffer += numPushed;
    // todo: this isn't safe - if an interrupt pops a byte right now, right before this next line, we'll miss this byte! Not a catastrophe because when a second byte gets popped we'll catch up, but could definitely be improved
    txQueue.waitForHeadChange();
    numPushed = tx_nonBlocking(buffer, len);
  }
}
void uart::tx(uint8_t byte) {
  tx(&byte, 1);
}

size_t uart::txQueueLen() {
  noInterrupts();
  const uint8_t len = txQueue.len();
  interrupts();
  return len;
}
size_t uart::txMaxQueueLen() {
  return txQueue.maxLen();
}