#ifndef DUBINS_PATHS_H
#define DUBINS_PATHS_H

#include "dublin.h"
#include <vector>

struct RankedPath {
  const char* name;
  Path path;
};

std::vector<RankedPath> allDubinsPaths(Point start, Point end, int R);

#endif
