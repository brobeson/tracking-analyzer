#ifndef ANALYZER_TEST_UTILITIES_H
#define ANALYZER_TEST_UTILITIES_H

#include "tracking-analyzer/bounding_box.h"

namespace analyzer
{
  // I know comparing floating point data like this is generally unsafe, but it
  // works for the unit tests.
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wfloat-equal"
  [[nodiscard]] inline auto operator==(const analyzer::bounding_box& a,
                                       const analyzer::bounding_box& b)
  {
    return a.x == b.x && a.y == b.y && a.width == b.width
           && a.height == b.height;
  }
#pragma GCC diagnostic pop

}  // namespace analyzer

#endif
