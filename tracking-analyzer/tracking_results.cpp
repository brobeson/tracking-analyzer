#include "tracking_results.h"
#include "dataset.h"
#include <QDir>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <iostream>

namespace analyzer
{
  namespace
  {
    auto read_sequence_names(const QString& results_path)
    {
      const QDir directory {results_path};
      return directory.entryList({"*.txt"}, QDir::Files, QDir::Name);
    }

    auto read_json_data(const QString& filepath)
    {
      QFile file {filepath};
      if (!file.open(QIODevice::ReadOnly))
      {
        throw std::runtime_error {"cannot open file"};
      }
      return QJsonDocument::fromJson(file.readAll()).object();
    }

    auto parse_training_score_update_frames(const QJsonObject& score_json)
    {
      std::vector<int> update_frames;
      const auto keys {score_json.keys()};
      std::transform(std::begin(keys),
                     std::end(keys),
                     std::back_insert_iterator {update_frames},
                     [](const QString& key) { return key.toInt(); });
      return update_frames;
    }

    auto parse_training_scores(const QJsonObject& score_json)
    {
      for (const auto& update : score_json)
      {
        const auto update_scores {update.toArray()};
        for (const auto& scores : update_scores)
        {
          const auto score_object {scores.toObject()};
        }
      }
    }
  }  // namespace

  auto load_tracking_results(const QString& path)
    -> std::vector<tracking_results>
  {
    const auto absolute_path {make_absolute_path(path)};
    const auto sequences {analyzer::read_sequence_names(absolute_path)};
    return std::vector<tracking_results> {};
  }

  auto load_training_scores(const QString& path) -> training_scores
  {
    const auto absolute_path {make_absolute_path(path)};
    const auto json_data {read_json_data(absolute_path)};
    const auto score_json {json_data["data"].toObject()};
    training_scores score_data {json_data["sequence"].toString(),
                                json_data["dataset"].toString(),
                                parse_training_score_update_frames(score_json),
                                {}};
    parse_training_scores(score_json);
    return score_data;
  }
}  // namespace analyzer
