#ifndef ANALYZER_TRACKING_RESULTS_H
#define ANALYZER_TRACKING_RESULTS_H

#include "tracking-analyzer/bounding_box.h"
#include <QList>
#include <QString>
#include <array>
#include <utility>
#include <vector>

namespace analyzer
{
  struct tracking_results
  {
    QString sequence_name;
    std::vector<analyzer::bounding_box> bounding_boxes;
  };

  [[nodiscard]] auto load_tracking_results(const QString& path)
    -> std::vector<tracking_results>;
}  // namespace analyzer

#endif