#pragma once

#include <stdint.h>
#include <limits.h>

namespace cfg {

// ===== HX711 =====
constexpr uint8_t  HX711_MEDIAN_WINDOW  = 5;
constexpr uint32_t HX711_STALE_MS       = 1500;  // 10 SPS 模式下留足上电与采样余量

// ===== 双点标定 =====
// 2026-11-08 实测，均为 30 秒内中位数的均值：100% 为空桶（30 个样本），参考点为剩余 15%（32 个样本）。
constexpr int32_t CAL_RAW_100 = 584098;
constexpr int32_t CAL_RAW_REF = 643833;
constexpr float   CAL_PCT_REF = 15.0f;

// ===== WS2812B-8 =====
constexpr uint8_t LED_COUNT      = 8;
constexpr uint8_t LED_BRIGHTNESS = 40;            // 0-255，避免 AMS1117 发烫

// ===== 显示节流 =====
constexpr float    DISPLAY_DEAD_BAND_PCT = 2.0f;
constexpr uint32_t DISPLAY_REFRESH_MS    = 250;

// ===== 主调度 =====
constexpr uint32_t LOOP_TICK_MS = 20;

}  // namespace cfg
