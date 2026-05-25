using namespace std;
#include <vector>

#ifndef DUBLIN_H
#define DUBLIN_H

enum class SegmentType { Left, Right, Straight };

enum class DubinsType { LSL, RSR, LSR, RSL, RLR, LRL };

struct Point {
  int x;
  int y;
  float deg;
};
struct Segment {
  SegmentType type;
  double length;
};

struct Path {
  std::vector<Segment> segments;
  double length;
};

Path makeDublin(Point start, Point end, int R, DubinsType type);

#endif
