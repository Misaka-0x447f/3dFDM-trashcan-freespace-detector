#include "WeightMapper.h"

#include <math.h>
#include <stdlib.h>

namespace {
constexpr float kEmaAlpha = 0.3f;
}

WeightMapper::WeightMapper(float deadBandPct)
    : _raw100(0), _rawRef(0), _pctRef(0), _deadBand(deadBandPct), _filtered(NAN),
      _calibrated(false), _hasValue(false), _displayPercent(kInvalid) {}

void WeightMapper::setCalibration(int32_t raw100, int32_t rawRef, float pctRef) {
  _raw100 = raw100;
  _rawRef = rawRef;
  _pctRef = pctRef;
  _calibrated = raw100 != INT32_MIN && rawRef != INT32_MIN && raw100 != rawRef && pctRef < 100.0f;
  _hasValue = false;
  _filtered = NAN;
  _displayPercent = kInvalid;
}

void WeightMapper::feed(int32_t raw) {
  if (!_calibrated) return;
  if (!_hasValue) {
    _filtered = static_cast<float>(raw);
    _hasValue = true;
  } else {
    _filtered = (1.0f - kEmaAlpha) * _filtered + kEmaAlpha * static_cast<float>(raw);
  }

  // 100% 点和参考点定义直线；区间外沿同一斜率外推。
  const float pct = 100.0f - (100.0f - _pctRef) *
      (_filtered - static_cast<float>(_raw100)) /
      (static_cast<float>(_rawRef) - static_cast<float>(_raw100));
  if (pct >= INT16_MAX) {
    _displayPercent = INT16_MAX;
    return;
  }
  if (pct <= INT16_MIN + 1) {
    _displayPercent = INT16_MIN + 1;  // INT16_MIN 留给无效态
    return;
  }
  const int16_t rounded = static_cast<int16_t>(pct + (pct >= 0 ? 0.5f : -0.5f));
  if (_displayPercent == kInvalid ||
      abs(static_cast<int>(rounded) - _displayPercent) >= _deadBand) {
    _displayPercent = rounded;
  }
}

void WeightMapper::invalidate() {
  _hasValue = false;
  _filtered = NAN;
  _displayPercent = kInvalid;
}
