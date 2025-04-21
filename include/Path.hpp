//
// Created by cervi on 21/04/2025
//

#ifndef PATH_HPP
#define PATH_HPP

#include <Engine/math.hpp>
#include <nds.h>
#include <vector>

class Path {
public:
  Path(std::vector<std::pair<s32, s32>> positions, bool smooth, bool closed);
  std::pair<s32, s32> advance(s32 speed);

private:
  static constexpr size_t kSplineResolution = 5;
  void createSplineInterpolations(std::vector<std::pair<s32, s32>> pos);
  void createPosLookup();
  std::vector<std::pair<s32, s32>> _posSpline;
  std::vector<s32> _dst;

  size_t _currentIdx;
  s32 _currentDst;
  bool _closed;
};

#endif
