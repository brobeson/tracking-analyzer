#ifndef ANALYZER_TRAINING_METADATA_H
#define ANALYZER_TRAINING_METADATA_H

#include <QList>

namespace analyzer
{
  using score_list = QList<QPointF>;

  struct training_iteration
  {
    score_list background_candidates;
    score_list background_mined;
    score_list target_candidates;
    float background_threshold;
    float target_threshold;
  };

  struct training_scores
  {
    static constexpr int background_scores {0};
    static constexpr int target_scores {1};
    QString sequence_name;
    QString dataset;
    std::vector<int> update_frames;
    training_iteration iteration_scores;
  };

  [[nodiscard]] auto load_training_scores(const QString& path)
    -> training_scores;

  using range = std::pair<float, float>;
  auto get_chart_range(const training_iteration& iteration) -> range;
}  // namespace analyzer

#endif
