#pragma once

#include <stdint.h>

class Hx711 {
public:
  Hx711(uint8_t dtPin, uint8_t sckPin);
  void begin();
  bool ready() const;
  // ready() 为 true 后调用；返回 24 位二补码的符号扩展值。
  int32_t read();

private:
  uint8_t _dtPin;
  uint8_t _sckPin;
};
