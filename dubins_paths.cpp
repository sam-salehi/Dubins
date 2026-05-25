#include "dubins_paths.h"
#include <algorithm>

static const char* dubinsName(DubinsType type) {
  switch (type) {
  case DubinsType::LSL:
    return "LSL";
  case DubinsType::RSR:
    return "RSR";
  case DubinsType::LSR:
    return "LSR";
  case DubinsType::RSL:
    return "RSL";
  case DubinsType::RLR:
    return "RLR";
  case DubinsType::LRL:
    return "LRL";
  }
  return "?";
}

std::vector<RankedPath> allDubinsPaths(Point start, Point end, int R) {
  const DubinsType types[] = {DubinsType::LSL, DubinsType::RSR, DubinsType::LSR,
                              DubinsType::RSL, DubinsType::RLR, DubinsType::LRL};

  std::vector<RankedPath> paths;
  for (DubinsType type : types) {
    Path path = makeDublin(start, end, R, type);
    if (path.segments.size() == 3) {
      paths.push_back({dubinsName(type), path});
    }
  }

  std::sort(paths.begin(), paths.end(),
            [](const RankedPath& a, const RankedPath& b) {
              return a.path.length < b.path.length;
            });
  return paths;
}
