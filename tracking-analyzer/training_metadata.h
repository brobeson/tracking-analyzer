#ifndef ANALYZER_TRAINING_METADATA_H
#define ANALYZER_TRAINING_METADATA_H

#include <QList>
#include <vector>

namespace analyzer
{
  using score_list = QList<QPointF>;

  struct training_batch
  {
    score_list background_candidates;
    score_list background_mined;
    score_list target_candidates;
    float background_threshold;
    float target_threshold;
  };

  using training_update = std::vector<training_batch>;

  struct training_scores
  {
    static constexpr int background_scores {0};
    static constexpr int target_scores {1};
    QString sequence_name;
    QString dataset;
    std::vector<int> update_frames;
    training_update update;
  };

  struct training_iterator
  {
    training_update::size_type current_batch {0};
  };

  [[nodiscard]] auto load_training_scores(const QString& path)
    -> training_scores;

  using range = std::pair<float, float>;
  auto get_chart_range(const training_batch& batch) -> range;
}  // namespace analyzer

#endif
