#ifndef ANALYZER_TRACKING_RESULTS_H
#define ANALYZER_TRACKING_RESULTS_H

#include "bounding_box.h"
#include <QString>
#include <array>
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

  struct training_scores
  {
    QString sequence_name;
    QString dataset;
    std::vector<int> update_frames;
    using score_list = std::vector<float>;
    using batch_scores = std::array<score_list, 2>;
    using update_scores = std::array<batch_scores, 3>;
    std::vector<update_scores> score_data;
  };

  [[nodiscard]] auto load_training_scores(const QString& path)
    -> training_scores;
}  // namespace analyzer

#endif