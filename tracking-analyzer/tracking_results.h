#ifndef ANALYZER_TRACKING_RESULTS_H
#define ANALYZER_TRACKING_RESULTS_H

#include "bounding_box.h"
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

  using score_list = QList<QPointF>;
  using range = std::pair<float, float>;

  struct training_iteration
  {
    score_list background_candidates;
    score_list background_mined;
    score_list target_candidates;
  };
  auto get_chart_range(const training_iteration& iteration) -> range;

  struct training_scores
  {
    // score_data := [update_scores, ...]
    // update := [batch_scores (bg candidates),
    //            batch_scores (bg batch),
    //            batch_scores (tg batch)]
    // batch_scores := [score_list (background), score_list (target)]
    // score_list := [float, ...]
    static constexpr int background_scores {0};
    static constexpr int target_scores {1};
    QString sequence_name;
    QString dataset;
    std::vector<int> update_frames;
    // using score_list = std::vector<double>;
    // using batch_scores = std::array<score_list, 2>;
    // using update = std::array<batch_scores, 3>;
    // std::vector<update> score_data;
    // using one_iteration = std::tuple<score_list, score_list, score_list>;
    // one_iteration score_data;
    training_iteration iteration_scores;
  };

  [[nodiscard]] auto load_training_scores(const QString& path)
    -> training_scores;
}  // namespace analyzer

#endif