#pragma once

class UartInternal {
public:
  static bool isTxActive();
  static bool isRxActive();

  static uint16_t cyclesPerBit;
  static uint8_t rxBitsPerFrame;

  // initializes the uart tx and rx setup
  static void begin(uint32_t baud);

  // Queues the contents of the buffer to be sent over UART. If the queue is too full to take the entire buffer, only some of it is queued. Returns how many bytes were queued.
  static size_t tx_nonBlocking(uint8_t* buffer, size_t len);

  // Queues the contents of the buffer to be sent over UART. If the queue is too full to take the entire buffer, blocks until the entire message can be queued.
  static void tx(uint8_t* buffer, size_t len);

  // Queues the byte to be sent over UART. If the queue is too full to take the byte, blocks until the byte can be queued.
  static void tx(uint8_t byte);

  // Returns the number of characters currently queued to be sent over uart, and the maximum
  static size_t txQueueLen();
  static size_t txMaxQueueLen();

  // Reads a byte from the uart rx queue. If the uart rx queue is empty, returns false and leaves outByte unmodified
  static bool readByte_nonBlocking(uint8_t& outByte);

  // Reads a byte from the uart rx queue. If the uart rx queue is empty, blocks until a byte is available
  static uint8_t readByte();

  // Returns the number of characters currently buffered after being received from uart, and the maximum
  static size_t rxQueueLen();
  static size_t rxMaxQueueLen();
};
