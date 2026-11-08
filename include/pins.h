#pragma once

#include <stdint.h>

// 默认：标准 NodeMCU 丝印映射
//   D5 = GPIO14 = SCL
//   D6 = GPIO12 = SDA
// 若 OLED 不亮，在 platformio.ini 改用 -DI2C_PINMAP_SWAPPED 重烧。
#if defined(I2C_PINMAP_SWAPPED)
constexpr uint8_t PIN_I2C_SCL = 12;  // D5（你自报）
constexpr uint8_t PIN_I2C_SDA = 14;  // D6（你自报）
#else
constexpr uint8_t PIN_I2C_SCL = 14;  // D5（NodeMCU 标准）
constexpr uint8_t PIN_I2C_SDA = 12;  // D6（NodeMCU 标准）
#endif

// HX711：NodeMCU D1/D2 使用标准 GPIO 映射，不受 OLED D5/D6 特例影响。
constexpr uint8_t PIN_HX711_DT  = 5;  // D1
constexpr uint8_t PIN_HX711_SCK = 4;  // D2

// WS2812B-8
constexpr uint8_t PIN_LED_DIN = 13;  // D7
