#pragma once

#include <stddef.h>
#include <stdint.h>

// 距离 → 剩余空间百分比（多段线性拟合）
// pcts[] 与 dists[] 一一对应、按距离升序排列；段内线性插值，段外用首/末段斜率外推。
// 不做范围 clamp：超出标定区间会输出 <0 或 >100，由下游决定怎么显示。
class LevelMapper {
public:
  static constexpr int16_t kInvalid = INT16_MIN;

  LevelMapper(const float* pcts, const float* dists, size_t count, float deadBandPct);

  // 输入一次中位数后的距离 (cm)；NAN 表示本轮无效，跳过更新。
  void feed(float distanceCm);

  // 显示用百分比（带死区滞回）。尚无有效值时返回 kInvalid。
  int16_t percent() const { return _displayPercent; }

  // 最近一次内部平滑后的距离（cm）；无值返回 NAN。
  float  filteredDistance() const { return _filtered; }

private:
  float distanceToPercent(float d) const;

  const float* _pcts;
  const float* _dists;
  size_t       _count;
  float        _deadBand;
  float        _filtered;        // EMA
  bool         _hasValue;
  int16_t      _displayPercent;  // kInvalid = uninitialized
};
