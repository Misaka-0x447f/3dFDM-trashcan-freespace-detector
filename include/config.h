#pragma once

#include <stdint.h>
#include <stddef.h>

namespace cfg {

// ===== HC-SR04 多段线性标定 =====
// 剩余空间百分比 → 探头到料面距离 (cm)，按距离升序排列。
// 拟合算法在两个相邻点之间线性插值；段外用首/末段斜率外推（允许结果 <0 或 >100）。
// 桶形非规则时多加几个中间点拟合得更准。HC-SR04 盲区 2cm，第一段距离 ≥3cm 余量。
constexpr float CAL_PCT[]  = {   0.0f,  25.0f,  50.0f,  75.0f, 100.0f };
constexpr float CAL_DIST[] = {  10.0f,  12.0f,  16.0f,  18.0f,  20.3f };
constexpr size_t CAL_COUNT = sizeof(CAL_PCT) / sizeof(CAL_PCT[0]);
static_assert(sizeof(CAL_PCT) == sizeof(CAL_DIST), "CAL_PCT / CAL_DIST length mismatch");

// ===== HC-SR04 工作参数 =====
constexpr uint32_t US_TIMEOUT_US        = 25000;  // ~4m 往返；避免丢回波卡 1s
constexpr uint32_t US_PING_INTERVAL_MS  = 80;     // 两次 ping 间距，≥60ms 防回波污染
constexpr size_t   US_MEDIAN_WINDOW     = 5;      // 中位数窗口
constexpr float    US_VALID_MIN_CM      = 2.0f;   // 盲区
constexpr float    US_VALID_MAX_CM      = 400.0f;

// ===== WS2812B-8 =====
constexpr uint8_t LED_COUNT      = 8;
constexpr uint8_t LED_BRIGHTNESS = 40;            // 0-255，避免 AMS1117 发烫

// ===== 显示节流 =====
constexpr float    DISPLAY_DEAD_BAND_PCT = 2.0f;  // 百分比变化 <= 2% 不刷
constexpr uint32_t DISPLAY_REFRESH_MS    = 250;

// ===== 主调度 =====
constexpr uint32_t LOOP_TICK_MS = 20;

}  // namespace cfg
