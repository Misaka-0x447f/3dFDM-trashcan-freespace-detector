#pragma once

#include <U8g2lib.h>
#include <stdint.h>

class OledView {
public:
  static constexpr int16_t kInvalid = INT16_MIN;

  // 引脚由编译期常量从 pins.h 注入。SW_I2C 直接拿到引脚，
  // 不依赖 Wire 默认 SDA/SCL，杜绝被库内 Wire.begin() 覆盖的隐患。
  OledView(uint8_t scl, uint8_t sda);

  void begin();

  // R2 旋转 + 双色屏物理分区布局：
  //   黄区（用户视觉 y=48..63）→ 距离 xx.x cm（靠右）
  //   蓝区（用户视觉 y=0..45） → 大百分比 + %（靠右，允许 <0 / >100）
  // 节流：百分比或距离变化（0.1cm 颗粒度）才刷。kInvalid 表示无效态。
  void render(int16_t percent, float distanceCm);

private:
  U8G2_SSD1306_128X64_NONAME_F_SW_I2C _u8g2;
  int16_t _lastPercent;
  int16_t _lastDistDeci;  // 距离×10；INT16_MIN 表示 NAN
};
