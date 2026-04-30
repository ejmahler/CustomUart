#include <Arduino.h>
#include "uart.h"
#include "uart_internal.h"
#include "pins.h"
#include "ring_buffer.h"

// ========================================================================================
// RECEIVE (aka incoming)
// ========================================================================================
volatile uint16_t rxFrame = 0; // The current frame being received. Contains more than 8 bits because we also put the stop bit(s) and parity bit (if configured) in here
volatile uint8_t rxFrameIndex = 0;
RingBuffer<5> rxQueue;

// A transmit is active if OCR1B interrupts are enabled
bool UartInternal::isRxActive() {
  return (TIMSK1 & (1 << OCIE1B)) != 0;
}

void endRx() {
  // shut off OCR1B interrupts
  const uint8_t interrupts = TIMSK1;
  TIMSK1 = interrupts & ~(1 << OCIE1B);
}

// Triggers on the falling edge of our RX pin. If we aren't executing an RX, this starts one. If we are, it tweaks the timings to keep up with the sender
ISR(TIMER1_CAPT_vect) {
  const uint16_t fallTime = ICR1;
  if(!UartInternal::isRxActive()) {
    // enable OCR1B interrupts
    const uint8_t interrupts = TIMSK1;
    TIMSK1 = interrupts | (1 << OCIE1B);

    // If an OCR1B interrupt is scheduled, cancel it
    TIFR1 |= 1 << OCF1B;

    rxFrameIndex = 0;    
  } else {
    // A rx is already active
  }
  
  // Schedule the next event half a bit after the falling edge. This will let us sample each bit in the middle of each bit
  // Note that we do this even when we're not starting a new rx! This lets us adapt if the transmitter isn't transmitting at exactly the right baud rate
  OCR1B = fallTime + UartInternal::cyclesPerBit / 2;
}

// Executes once for each bit we receive
ISR(TIMER1_COMPB_vect) {
  const bool rxPinValue = readRx();
  uint8_t localFrameIndex = rxFrameIndex;
  OCR1B += UartInternal::cyclesPerBit;

  if(localFrameIndex == 0) {
    // The first bit is special - if it isn't a 0, we're going to assume that the transmission is finished
    if(rxPinValue) {
      endRx();
      return;
    }
    rxFrameIndex = localFrameIndex + 1;
  } else {
    uint16_t localFrame = rxFrame;
    localFrame |= static_cast<uint16_t>(rxPinValue) << localFrameIndex;
    
    // If haven't received all bits for this frame, just write back
    localFrameIndex++;
    if(localFrameIndex < UartInternal::rxBitsPerFrame) {
      rxFrame = localFrame;
      rxFrameIndex = localFrameIndex;
    } else {
      // We've received all bits. Validate and push into the rx buffer
      // If the stop bit isn't 1, discard this frame and shut off the Rx interrupts until it goes high again. This will happen automatically when TIMER1_CAPT_vect triggers
      if((localFrame & (1 << (UartInternal::rxBitsPerFrame - 1))) == 0) {
        endRx();
      } else {
        // This is a valid byte, so push it
        rxQueue.pushOverwrite(static_cast<uint8_t>(localFrame >> 1));
      }
      
      // Set up for next frame
      rxFrame = 0;
      rxFrameIndex = 0;
    }
  }
}

bool UartInternal::readByte_nonBlocking(uint8_t& outByte) {
  noInterrupts();
  const bool result = rxQueue.popNonBlocking(outByte);
  interrupts();
  return result;
}

uint8_t UartInternal::readByte() {
  uint8_t result;
  while(!readByte_nonBlocking(result)) {
    // todo: this isn't safe - if an interrupt pushes a byte right now, between these two lines, we'll miss this byte! Not a catastrophe because when a second byte gets pushed we'll catch up, but could definitely be improved
    rxQueue.waitForTailChange();
  }
  return result;
}

size_t UartInternal::rxQueueLen() {
  noInterrupts();
  const uint8_t len = rxQueue.len();
  interrupts();
  return len;
}
size_t UartInternal::rxMaxQueueLen() {
  return rxQueue.maxLen();
}
