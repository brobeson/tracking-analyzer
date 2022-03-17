#include "training_samples.h"
#include "filesystem.h"
#include <QDir>
#include <QJsonArray>
#include <QJsonObject>
#include <QString>

namespace analyzer
{
  namespace
  {
    [[nodiscard]] auto parse_training_frame(const QJsonValue& json_data)
    {
      const auto json_object = json_data.toObject();
      frame_samples frame {json_object["frame_number"].toInt(), {}};
      return frame;
    }

    // [[nodiscard]] auto parse_sequence(const std::string& sequence_name,
    //                                   const QJsonValue& json_data)
    // {
    //   const auto& sequence_object {json_data.toObject()};
    //   sequence_training_samples sequence {sequence_name, {}};
    //   return sequence;
    // }

    // [[nodiscard]] auto parse_training_samples(const QJsonArray& json_data)
    // {
    //   training_sample_list samples;
    //   // std::transform(std::begin(json_data),
    //   //                std::end(json_data),
    //   //                std::back_inserter(samples),
    //   //                &parse_sequence);
    //   return samples;
    // }

    [[nodiscard]] auto sequence_name_from_path(QString path)
    {
      return path.remove(0, path.lastIndexOf('/') + 1)
        .remove("_training_samples.json");
    }

    [[nodiscard]] auto load_training_samples_for_sequence(const QString& path)
    {
      sequence_training_samples sequence_db {sequence_name_from_path(path), {}};
      const auto json_doc {read_json_data(path)};
      const auto json_data = json_doc.array();
      std::transform(std::begin(json_data),
                     std::end(json_data),
                     std::back_inserter(sequence_db.frames),
                     &parse_training_frame);
      return sequence_db;
    }

    [[nodiscard]] auto
    load_training_samples_for_tracker(const QString& path,
                                      const QString& tracker_name)
    {
      const auto sequences {get_sequence_file_paths(path + '/' + tracker_name,
                                                    "_training_samples.json")};
      tracker_training_samples db {tracker_name, {}};
      std::transform(std::begin(sequences),
                     std::end(sequences),
                     std::back_inserter(db.sequences),
                     [&path, &tracker_name](const QString& sequence) {
                       return load_training_samples_for_sequence(
                         path + '/' + tracker_name + '/' + sequence);
                     });
      return db;
    }
  }  // namespace

  auto load_training_samples(const QString& path) -> training_sample_database
  {
    const auto metadata_path {make_absolute_path(path)};
    const auto trackers {get_subdirectories(metadata_path)};
    training_sample_database db;
    std::transform(std::begin(trackers),
                   std::end(trackers),
                   std::back_inserter(db.m_trackers),
                   [&metadata_path](const auto& tracker) {
                     return load_training_samples_for_tracker(metadata_path,
                                                              tracker);
                   });
    // return parse_training_samples(json_data);
    // return sequence_training_samples {sequence_name_from_path(path), {}};
    return db;
  }

  auto get_trackers_in_database(const training_sample_database& db)
    -> QStringList
  {
    QStringList trackers;
    std::transform(std::cbegin(db.m_trackers),
                   std::cend(db.m_trackers),
                   std::back_inserter(trackers),
                   [](const tracker_training_samples& r) { return r.name; });
    return trackers;
  }

  auto get_tracker_results(const training_sample_database& db,
                           const QString& tracker_name)
    -> tracker_training_samples
  {
    const auto i {
      std::find_if(std::begin(db.m_trackers),
                   std::end(db.m_trackers),
                   [&tracker_name](const tracker_training_samples& candidate) {
                     return candidate.name == tracker_name;
                   })};
    if (i == db.m_trackers.end())
    {
      throw std::runtime_error {
        "Tracker not found in training samples database."};
    }
    return *i;
  }
}  // namespace analyzer
