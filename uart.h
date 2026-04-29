#pragma once

namespace uart {
  // initializes the uart tx and rx setup
  void begin(uint32_t baud);

  // Queues the contents of the buffer to be sent over UART. If the queue is too full to take the entire buffer, only some of it is queued. Returns how many bytes were queued.
  size_t tx_nonBlocking(uint8_t* buffer, size_t len);

  // Queues the contents of the buffer to be sent over UART. If the queue is too full to take the entire buffer, blocks until the entire message can be queued.
  void tx(uint8_t* buffer, size_t len);

  // Queues the byte to be sent over UART. If the queue is too full to take the byte, blocks until the byte can be queued.
  void tx(uint8_t byte);

  // Returns the number of characters currently queued to be sent over uart, and the maximum
  size_t txQueueLen();
  size_t txMaxQueueLen();

  // Reads a byte from the uart rx queue. If the uart rx queue is empty, returns false and leaves outByte unmodified
  bool readByte_nonBlocking(uint8_t& outByte);

  // Reads a byte from the uart rx queue. If the uart rx queue is empty, blocks until a byte is available
  uint8_t readByte();

  // Returns the number of characters currently buffered after being received from uart, and the maximum
  size_t rxQueueLen();
  size_t rxMaxQueueLen();
}