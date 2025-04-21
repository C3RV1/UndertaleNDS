//
// Created by cervi on 21/04/2025
//

#include "Path.hpp"

Path::Path(std::vector<std::pair<s32, s32>> positions, bool smooth,
           bool closed) {
  _closed = closed;

  if (smooth)
    createSplineInterpolations(std::move(positions));
  else
    _posSpline = std::move(positions);

  if (_closed)
    _posSpline.push_back(_posSpline[0]);

  createPosLookup();

  _currentIdx = 0;
  _currentDst = 0;
}

#include <string>

void Path::createSplineInterpolations(std::vector<std::pair<s32, s32>> pos) {
  size_t posSplineSize = pos.size() * kSplineResolution;

  _posSpline.reserve(posSplineSize);

  size_t p_prev, p_current, p_next;
  std::pair<s32, s32> c1, c2, p1, p2;
  for (p_current = 0; p_current < pos.size(); p_current++) {
    if (p_current == 0)
      p_prev = pos.size() - 1;
    else
      p_prev = p_current - 1;
    p_next = (p_current + 1) % pos.size();

    auto &prevPos = pos[p_prev];
    auto &currentPos = pos[p_current % pos.size()];
    auto &nextPos = pos[p_next];

    // Spline control points between points
    c1.first = currentPos.first + (prevPos.first - currentPos.first) / 2;
    c1.second = currentPos.second + (prevPos.second - currentPos.second) / 2;
    c2.first = currentPos.first + (nextPos.first - currentPos.first) / 2;
    c2.second = currentPos.second + (nextPos.second - currentPos.second) / 2;

    if (p_current == 0 && !_closed) {
      for (size_t i = 0; i < kSplineResolution; i++)
        _posSpline.push_back(lerp(currentPos, c2, i, kSplineResolution));
      continue;
    }

    if (p_current == pos.size() - 1 && !_closed) {
      for (size_t i = 0; i < kSplineResolution; i++)
        _posSpline.push_back(lerp(c1, currentPos, i, kSplineResolution));
      continue;
    }

    for (size_t i = 0; i < kSplineResolution; i++) {
      p1 = lerp(c1, currentPos, i, kSplineResolution);
      p2 = lerp(currentPos, c2, i, kSplineResolution);
      _posSpline.push_back(lerp(p1, p2, i, kSplineResolution));
    }
  }
}

void Path::createPosLookup() {
  _dst.reserve(_posSpline.size() - 1);

  std::string buffer;
  for (size_t i = 0; i < _posSpline.size() - 1; i++) {
    auto &p1 = _posSpline[i];
    auto &p2 = _posSpline[i + 1];
    s32 dstX = (p1.first - p2.first);
    s32 dstY = (p1.second - p2.second);
    s32 dstSqr = dstX * dstX + dstY * dstY;
    _dst.push_back(sqrt32(dstSqr));
  }
}

std::pair<s32, s32> Path::advance(s32 speed) {
  if (_dst.empty())
    return {0, 0};
  _currentDst += speed;
  while (_currentDst > _dst[_currentIdx]) {
    _currentDst -= _dst[_currentIdx];
    _currentIdx = (_currentIdx + 1) % _dst.size();
  }

  auto &p1 = _posSpline[_currentIdx];
  auto &p2 = _posSpline[_currentIdx + 1];

  return lerp(p1, p2, _currentDst, _dst[_currentIdx]);
}
