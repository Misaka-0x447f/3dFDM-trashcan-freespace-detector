#pragma once

#include <stdint.h>

class WeightMapper {
public:
  static constexpr int16_t kInvalid = INT16_MIN;

  explicit WeightMapper(float deadBandPct);
  void setCalibration(int32_t raw100, int32_t rawRef, float pctRef);
  bool calibrated() const { return _calibrated; }
  void feed(int32_t raw);
  void invalidate();
  int16_t percent() const { return _displayPercent; }
  float filteredRaw() const { return _filtered; }

private:
  int32_t _raw100;
  int32_t _rawRef;
  float _pctRef;
  float _deadBand;
  float _filtered;
  bool _calibrated;
  bool _hasValue;
  int16_t _displayPercent;
};
