#include "dublin.h"
#include <cmath>

const float TWO_PI = 2.0f * static_cast<float>(M_PI);

struct Bearing {
  float d;
  float alpha;
  float beta;
};

struct Candidate {
  SegmentType types[3];
  float t;
  float p;
  float q;
  bool valid;
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

static Candidate makeCandidate(SegmentType s0, SegmentType s1, SegmentType s2,
                               float t, float p, float q, bool valid) {
  Candidate c;
  c.types[0] = s0;
  c.types[1] = s1;
  c.types[2] = s2;
  c.t = t;
  c.p = p;
  c.q = q;
  c.valid = valid;
  return c;
}

static Candidate lsl(float alpha, float beta, float d, float sa, float sb,
                     float ca, float cb, float c_ab) {
  float tmp0 = d + sa - sb;
  float p_sq = 2.f + d * d - 2.f * c_ab + 2.f * d * (sa - sb);
  if (p_sq < 0.f) {
    return makeCandidate(SegmentType::Left, SegmentType::Straight,
                         SegmentType::Left, 0.f, 0.f, 0.f, false);
  }
  float tmp1 = std::atan2(cb - ca, tmp0);
  return makeCandidate(SegmentType::Left, SegmentType::Straight,
                       SegmentType::Left, _mod2pi(-alpha + tmp1), std::sqrt(p_sq),
                       _mod2pi(beta - tmp1), true);
}

static Candidate rsr(float alpha, float beta, float d, float sa, float sb,
                     float ca, float cb, float c_ab) {
  float tmp0 = d - sa + sb;
  float p_sq = 2.f + d * d - 2.f * c_ab + 2.f * d * (sb - sa);
  if (p_sq < 0.f) {
    return makeCandidate(SegmentType::Right, SegmentType::Straight,
                         SegmentType::Right, 0.f, 0.f, 0.f, false);
  }
  float tmp1 = std::atan2(ca - cb, tmp0);
  return makeCandidate(SegmentType::Right, SegmentType::Straight,
                       SegmentType::Right, _mod2pi(alpha - tmp1), std::sqrt(p_sq),
                       _mod2pi(-beta + tmp1), true);
}

static Candidate lsr(float alpha, float beta, float d, float sa, float sb,
                     float ca, float cb, float c_ab) {
  float p_sq = -2.f + d * d + 2.f * c_ab + 2.f * d * (sa + sb);
  if (p_sq < 0.f) {
    return makeCandidate(SegmentType::Left, SegmentType::Straight,
                         SegmentType::Right, 0.f, 0.f, 0.f, false);
  }
  float p = std::sqrt(p_sq);
  float tmp2 = std::atan2(-ca - cb, d + sa + sb) - std::atan2(-2.f, p);
  return makeCandidate(SegmentType::Left, SegmentType::Straight,
                       SegmentType::Right, _mod2pi(-alpha + tmp2), p,
                       _mod2pi(-beta + tmp2), true);
}

static Candidate rsl(float alpha, float beta, float d, float sa, float sb,
                     float ca, float cb, float c_ab) {
  float p_sq = d * d - 2.f + 2.f * c_ab - 2.f * d * (sa + sb);
  if (p_sq < 0.f) {
    return makeCandidate(SegmentType::Right, SegmentType::Straight,
                         SegmentType::Left, 0.f, 0.f, 0.f, false);
  }
  float p = std::sqrt(p_sq);
  float tmp2 = std::atan2(ca + cb, d - sa - sb) - std::atan2(2.f, p);
  return makeCandidate(SegmentType::Right, SegmentType::Straight,
                       SegmentType::Left, _mod2pi(alpha - tmp2), p,
                       _mod2pi(beta - tmp2), true);
}

static Candidate rlr(float alpha, float beta, float d, float sa, float sb,
                     float ca, float cb, float c_ab) {
  float tmp0 = (6.f - d * d + 2.f * c_ab + 2.f * d * (sa - sb)) / 8.f;
  if (std::abs(tmp0) > 1.f) {
    return makeCandidate(SegmentType::Right, SegmentType::Left,
                         SegmentType::Right, 0.f, 0.f, 0.f, false);
  }
  float p = _mod2pi(TWO_PI - std::acos(tmp0));
  float tmp2 = alpha - std::atan2(ca - cb, d - sa + sb) + _mod2pi(p / 2.f);
  return makeCandidate(SegmentType::Right, SegmentType::Left,
                       SegmentType::Right, _mod2pi(tmp2), p,
                       _mod2pi(alpha - beta - tmp2 + _mod2pi(p)), true);
}

static Candidate lrl(float alpha, float beta, float d, float sa, float sb,
                     float ca, float cb, float c_ab) {
  float tmp0 = (6.f - d * d + 2.f * c_ab + 2.f * d * (sb - sa)) / 8.f;
  if (std::abs(tmp0) > 1.f) {
    return makeCandidate(SegmentType::Left, SegmentType::Right,
                         SegmentType::Left, 0.f, 0.f, 0.f, false);
  }
  float p = _mod2pi(TWO_PI - std::acos(tmp0));
  float tmp2 = -alpha - std::atan2(ca - cb, d + sa - sb) + _mod2pi(p / 2.f);
  return makeCandidate(SegmentType::Left, SegmentType::Right,
                       SegmentType::Left, _mod2pi(tmp2), p,
                       _mod2pi(beta - alpha - tmp2 + _mod2pi(p)), true);
}

static Path pathFromCandidate(const Candidate& c, int R) {
  Path path;
  float lengths[3] = {c.t, c.p, c.q};
  path.length = 0.0;
  for (int i = 0; i < 3; ++i) {
    path.segments.push_back({c.types[i], lengths[i]});
    path.length += static_cast<double>(lengths[i]) * R;
  }
  return path;
}

Path makeDublin(Point start, Point end, int R, DubinsType type) {
  Path empty;
  if (R <= 0) {
    return empty;
  }

  Bearing b = getBearing(start, end, R);
  float sa = std::sin(b.alpha);
  float sb = std::sin(b.beta);
  float ca = std::cos(b.alpha);
  float cb = std::cos(b.beta);
  float c_ab = std::cos(b.alpha - b.beta);

  Candidate c;
  switch (type) {
  case DubinsType::LSL:
    c = lsl(b.alpha, b.beta, b.d, sa, sb, ca, cb, c_ab);
    break;
  case DubinsType::RSR:
    c = rsr(b.alpha, b.beta, b.d, sa, sb, ca, cb, c_ab);
    break;
  case DubinsType::LSR:
    c = lsr(b.alpha, b.beta, b.d, sa, sb, ca, cb, c_ab);
    break;
  case DubinsType::RSL:
    c = rsl(b.alpha, b.beta, b.d, sa, sb, ca, cb, c_ab);
    break;
  case DubinsType::RLR:
    c = rlr(b.alpha, b.beta, b.d, sa, sb, ca, cb, c_ab);
    break;
  case DubinsType::LRL:
    c = lrl(b.alpha, b.beta, b.d, sa, sb, ca, cb, c_ab);
    break;
  }

  if (!c.valid) {
    return empty;
  }
  return pathFromCandidate(c, R);
}
