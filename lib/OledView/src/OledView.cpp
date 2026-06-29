#include "OledView.h"

#include <math.h>
#include <stdint.h>
#include <stdio.h>

namespace {
constexpr int  SCREEN_W = 128;
constexpr int  RIGHT_PAD_BLUE   = 0;  // 蓝区右边距
constexpr int  RIGHT_PAD_YELLOW = 5;  // 黄区距离行右边距（视觉左移）
constexpr int  GAP_NUM_PCT = -2;  // 数字和 % advance 重叠 2px；字符本身 padding 吸收，% 不动，数字再右移 3px

// 双色屏物理分区（U8G2_R2 旋转后的用户视觉坐标）：
//   黄区: vy = 48..63  （16 px 高）
//   黑条: vy = 46..47
//   蓝区: vy =  0..45  （46 px 高）
constexpr int Y_YELLOW_BASELINE = 58;   // 6x10 字体在黄区 baseline
constexpr int Y_PCT_BASELINE    = 42;   // 蓝区 % 符号 baseline
constexpr int Y_NUM_BASELINE    = 44;   // 蓝区大数字 baseline（视觉比 % 低 2 px）
}  // namespace

OledView::OledView(uint8_t scl, uint8_t sda)
    : _u8g2(U8G2_R2, scl, sda, U8X8_PIN_NONE),   // R2 = 180° 翻转
      _lastPercent(kInvalid + 1),
      _lastDistDeci(INT16_MIN + 1) {}

void OledView::begin() {
  _u8g2.begin();
  _u8g2.setBusClock(400000);
  _u8g2.clearBuffer();
  _u8g2.sendBuffer();
}

void OledView::render(int16_t percent, float distanceCm) {
  // 节流：百分比或距离（0.1cm 颗粒度）有变化才刷
  int16_t distDeci = isnan(distanceCm)
      ? INT16_MIN
      : static_cast<int16_t>(distanceCm * 10.0f + 0.5f);
  if (percent == _lastPercent && distDeci == _lastDistDeci) return;
  _lastPercent  = percent;
  _lastDistDeci = distDeci;

  _u8g2.clearBuffer();

  // ===== 黄区：距离 xx.x cm，靠右 =====
  _u8g2.setFont(u8g2_font_6x10_tr);
  char dbuf[16];
  if (isnan(distanceCm)) {
    snprintf(dbuf, sizeof(dbuf), "-- cm");
  } else {
    snprintf(dbuf, sizeof(dbuf), "%.1f cm", distanceCm);
  }
  int dw = _u8g2.getStrWidth(dbuf);
  _u8g2.drawStr(SCREEN_W - RIGHT_PAD_YELLOW - dw, Y_YELLOW_BASELINE, dbuf);

  // ===== 蓝区：大数字百分比 + %，靠右；允许 <0 / >100，无效态画 "--%" =====
  if (percent == kInvalid) {
    _u8g2.setFont(u8g2_font_helvB24_tr);
    const char* dashes = "--";
    int sw = _u8g2.getStrWidth(dashes);
    int pw = _u8g2.getStrWidth("%");
    int total = sw + GAP_NUM_PCT + pw;
    int x0 = SCREEN_W - RIGHT_PAD_BLUE - total;
    _u8g2.drawStr(x0, Y_NUM_BASELINE, dashes);
    _u8g2.drawStr(x0 + sw + GAP_NUM_PCT, Y_PCT_BASELINE, "%");
  } else {
    char num[8];
    snprintf(num, sizeof(num), "%d", static_cast<int>(percent));

    // logisoso38_tr 含 ASCII 含负号，支持 "-12" 等负数；比 _tn 多占 flash。
    _u8g2.setFont(u8g2_font_logisoso38_tr);
    int numW = _u8g2.getStrWidth(num);

    _u8g2.setFont(u8g2_font_helvB14_tr);
    int pctW = _u8g2.getStrWidth("%");

    int total = numW + GAP_NUM_PCT + pctW;
    int x0    = SCREEN_W - RIGHT_PAD_BLUE - total;

    // 先画 %（字体仍是上面 setFont 的 helvB14_tr）
    _u8g2.drawStr(x0 + numW + GAP_NUM_PCT, Y_PCT_BASELINE, "%");
    // 再切回大数字字体画百分数（视觉比 % 低 2 px）
    _u8g2.setFont(u8g2_font_logisoso38_tr);
    _u8g2.drawStr(x0, Y_NUM_BASELINE, num);
  }

  _u8g2.sendBuffer();
}
