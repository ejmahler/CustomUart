#pragma once

// Implements a ring buffer of bytes. Max length will be (1 << BITS) - 1
//
// WARNING: Interrupts must be disabled before calling all functions in this class except maxLen(), head(), and tail()
// The expectation is that both interrupt code and non-interrupt code will write to this class' fields
// So if interrupts are enabled, you may get torn reads if an interrupt is triggered while non-interrupt code is inside one of these functions
template<uint8_t BITS>
class RingBuffer {
public:
  // Returns the length of the ring buffer. 
  uint8_t len() const { return (tail - head) & mask(); }
  bool isEmpty() const { return tail == head; }

  // Returns the maximum possible length of the ring buffer
  constexpr uint8_t maxLen() const { return INTERNAL_BUFFER_LEN - 1; }

  // Pushes as many as possible from the provided buffer into the ring buffer. Returns how many were pushed.
  size_t pushGreedy(uint8_t* buffer, size_t len) {
    uint8_t copied = 0;

    // move variables local so they can sit in registers instead of having to be loaded every time
    uint8_t localTail = tail;
    uint8_t localHead = head;
    auto localLen = [&localTail, localHead] { return (localTail - localHead) & mask(); };

    while(localLen() < maxLen() && copied < len) {
      internalBuffer[localTail] = buffer[copied];
      localTail = (localTail + 1) & mask();
      copied++;
    }

    tail = localTail;

    return copied;
  }

  // Pushes a single byte into the queue. If the queue is full, the oldest byte is discarded.
  void pushOverwrite(uint8_t byte) {
    // move variables local so they can sit in registers instead of having to be loaded every time
    uint8_t localTail = tail;
    uint8_t localHead = head;

    internalBuffer[localTail] = byte;
    localTail = (localTail + 1) & mask();
    if(localHead == localTail) {
      head = (localHead + 1) & mask();
    }

    tail = localTail;
    ;
  }

  // Attempts to pop an item from the ring buffer. Returns false if the ring buffer is empty
  bool pop(uint8_t& byte) {
    uint8_t localTail = tail;
    uint8_t localHead = head;

    if(localTail != localHead) {
      byte = internalBuffer[localHead];
      head = (localHead + 1) & mask();
      return true;
    } else {
      return false;
    }
  }

  // Blocks until the head of the queue changes. Do not call while interrupts are disabled
  void waitForHeadChange() const { 
    const uint8_t startHead = head;
    while(head == startHead) {}
  }

  // Pops an item from the ring buffer. If the queue is empty, blocks until something is available
  void waitForTailChange() const { 
    const uint8_t startTail = tail;
    while(tail == startTail) {}
  }
private:
  constexpr static uint8_t mask() { return INTERNAL_BUFFER_LEN - 1; }

  constexpr static size_t INTERNAL_BUFFER_LEN = 1 << BITS;
  uint8_t internalBuffer[INTERNAL_BUFFER_LEN];
  volatile uint8_t head = 0; // points to the next element to be queued
  volatile uint8_t tail = 0; // points to one after the most recently queued element, aka the next free element. when head == tail, we're empty. When head == tail + 1, we're full
};