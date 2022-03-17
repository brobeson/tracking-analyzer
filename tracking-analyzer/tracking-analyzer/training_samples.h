#ifndef ANALYZER_TRAINING_SAMPLES_H
#define ANALYZER_TRAINING_SAMPLES_H

#include "bounding_box.h"
#include <QStringList>
#include <vector>

namespace analyzer
{
  struct synthetic_sample final
  {
    bounding_box p;
    bounding_box q;
    bounding_box synthetic;
    float distance {0.0f};
  };

  using synthetic_sample_list = std::vector<synthetic_sample>;

  struct frame_samples
  {
    int frame_number {0};
    synthetic_sample_list samples;
  };

  struct sequence_training_samples
  {
    QString name;
    std::vector<frame_samples> frames;
  };

  struct tracker_training_samples final
  {
    QString name;
    std::vector<sequence_training_samples> sequences;
  };

  struct training_sample_database final
  {
    std::vector<tracker_training_samples> m_trackers;
  };

  [[nodiscard]] auto load_training_samples(const QString& directory_path)
    -> training_sample_database;

  [[nodiscard]] auto
  get_trackers_in_database(const training_sample_database& db) -> QStringList;

  [[nodiscard]] auto get_tracker_results(const training_sample_database& db,
                                         const QString& tracker_name)
    -> tracker_training_samples;
}  // namespace analyzer

#endif
