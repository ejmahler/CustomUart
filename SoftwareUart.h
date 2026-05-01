#pragma once
#include <Stream.h>

class SoftwareUart : public Stream {
public:
  void begin(uint32_t baud);
  void end();

  // begin Print interface
  virtual size_t write(uint8_t byte) override;
  virtual size_t write(const uint8_t *buffer, size_t size) override;
  virtual int availableForWrite() override;
  virtual void flush() override;
  // end Print interface

  // begin Stream interface
  virtual int available() override;
  virtual int read() override;
  virtual int peek() override;
  // end Stream interface
};
