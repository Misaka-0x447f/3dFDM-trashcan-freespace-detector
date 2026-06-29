// 一次性 I2C 扫描固件。用法：
//   pio run -e i2c_scan -t upload && pio device monitor -b 115200
// 期望输出：found 0x3C（SSD1306 默认地址）。
// 看到地址后：
//   1) 把 platformio.ini 切回 default_envs = nodemcuv2 重烧主固件
//   2) 若扫不到地址，先把 platformio.ini 的 I2C_PINMAP_STANDARD 换成 I2C_PINMAP_SWAPPED 再扫一次
//   3) 两组映射都扫不到 → 线路/上拉/供电问题，不是软件

#include <Arduino.h>
#include <Wire.h>

#include "pins.h"

void setup() {
  Serial.begin(115200);
  delay(200);
  Serial.println();
  Serial.printf("[i2c-scan] SDA=GPIO%u  SCL=GPIO%u\n", PIN_I2C_SDA, PIN_I2C_SCL);
  Wire.begin(PIN_I2C_SDA, PIN_I2C_SCL);
  Wire.setClock(100000);
}

void loop() {
  Serial.println("[i2c-scan] sweep 0x03..0x77");
  uint8_t found = 0;
  for (uint8_t addr = 0x03; addr < 0x78; ++addr) {
    Wire.beginTransmission(addr);
    if (Wire.endTransmission() == 0) {
      Serial.printf("  found 0x%02X\n", addr);
      ++found;
    }
    delay(2);
  }
  Serial.printf("[i2c-scan] %u device(s)\n\n", found);
  delay(3000);
}
