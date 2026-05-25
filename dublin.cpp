#include "dublin.h"
#include <cmath>

const float TWO_PI = 2.0f * static_cast<float>(M_PI);

struct Bearing {
  float d;
  float alpha;
  float beta;
};

float _mod2pi(float angle) {

  angle = std::fmod(angle, TWO_PI);
  if (angle < 0.0f) {
    angle += TWO_PI;
  }

  return angle;
}

float _deg2pi(float deg) {
  float radian = deg / 360.0f * TWO_PI;
  return _mod2pi(radian);
}

Bearing getBearing(Point start, Point end, int R) {
  int dx = end.x - start.x;
  int dy = end.y - start.y;
  int D = sqrt(pow(dx, 2) + pow(dy, 2));
  float d = static_cast<float>(D) / R;
  float theta = std::atan2(dy, dx);

  float alpha = _mod2pi(_deg2pi(start.deg) - theta);
  float beta = _mod2pi(_deg2pi(end.deg) - theta);

  Bearing result = {d, alpha, beta};
  return result;
}

Path makeDublin(Point start, Point end, int R) {}
