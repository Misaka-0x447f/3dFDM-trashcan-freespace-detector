#include "LevelMapper.h"

#include <math.h>
#include <stdlib.h>

namespace {
constexpr float kEmaAlpha = 0.3f;  // 越大越灵敏；竹屎表面噪声大，留些惯性
}

LevelMapper::LevelMapper(const float* pcts, const float* dists, size_t count, float deadBandPct)
    : _pcts(pcts),
      _dists(dists),
      _count(count),
      _deadBand(deadBandPct),
      _filtered(NAN),
      _hasValue(false),
      _displayPercent(kInvalid) {}

void LevelMapper::feed(float distanceCm) {
  if (isnan(distanceCm)) return;

  // EMA 平滑（直接吃原始距离，不做边界 clamp）
  if (!_hasValue) {
    _filtered = distanceCm;
    _hasValue = true;
  } else {
    _filtered = (1.0f - kEmaAlpha) * _filtered + kEmaAlpha * distanceCm;
  }

  float pct = distanceToPercent(_filtered);
  int16_t p = static_cast<int16_t>(pct + (pct >= 0.0f ? 0.5f : -0.5f));

  // 死区滞回：变化必须 >= deadBand 才更新（首次直接装入）
  if (_displayPercent == kInvalid
      || abs(p - _displayPercent) >= static_cast<int>(_deadBand)) {
    _displayPercent = p;
  }
}

// 找到 d 所在或最接近的段 [i, i+1]，做线性插值（段外即外推）。
// 要求 _count >= 2 且 _dists 按升序排列。
float LevelMapper::distanceToPercent(float d) const {
  if (_count < 2) return 0.0f;  // 兜底；不会真的发生
  size_t i = 0;
  while (i + 2 < _count && d > _dists[i + 1]) ++i;
  const float dx = _dists[i + 1] - _dists[i];
  const float dy = _pcts [i + 1] - _pcts [i];
  return _pcts[i] + (d - _dists[i]) * dy / dx;
}
