#include "tracking_results.h"
#include "dataset.h"
#include <QDir>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <fstream>
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

    // auto parse_training_scores(
    //   const QJsonArray& score_data,
    //   training_scores::score_list::iterator background_inserter,
    //   training_scores::score_list::iterator target_inserter)
    // {
    //   for (const auto& datum : score_data)
    //   {
    //     const auto array {datum.toArray()};
    //     *background_inserter
    //       = array[training_scores::background_scores].toDouble();
    //     *target_inserter = array[training_scores::target_scores].toDouble();
    //     ++background_inserter;
    //     ++target_inserter;
    //   }
    // }

    // void fill_training_update(const QJsonArray& update_json_data,
    // std::vector)
    // auto parse_training_scores(const QJsonObject& score_json)
    // {
    //   std::vector<training_scores::update> score_data;
    //   for (const auto& update : score_json)
    //   {
    //     const auto update_scores {update.toArray()};
    //     for (const auto& scores : update_scores)
    //     {
    //       const auto score_object {scores.toObject()};
    //       score_data.emplace_back(training_scores::update {});
    //       auto json_iterator {score_object.find("background candidates")};
    //       if (json_iterator != score_object.end())
    //       {
    //         parse_training_scores(
    //           json_iterator->toArray(),
    //           std::back_insert_iterator {score_data.rend()->operator[](
    //             0)[training_scores::background_scores]},
    //           std::back_insert_iterator {score_data.rend()->operator[](
    //             0)[training_scores::target_scores]});
    //       }
    //     }
    //   }
    // }

    auto to_point(const QJsonValue& json)
    {
      // I'm not sure why, but something like QJsonArray a{json.toArray()};
      // doesn't work. Subsequent a[0].toDouble() returns 0, regardless of what
      // the actual value is.
      return QPointF {json.toArray()[0].toDouble(),
                      json.toArray()[1].toDouble()};
    }

    void fill_training_scores(const QJsonArray& json_data,
                              QList<QPointF>& scores)
    {
      std::transform(std::begin(json_data),
                     std::end(json_data),
                     std::back_insert_iterator(scores),
                     &to_point);
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
    fill_training_scores(score_json["0"]
                           .toArray()[0]
                           .toObject()["background candidates"]
                           .toArray(),
                         score_data.score_data.first);
    fill_training_scores(
      score_json["0"].toArray()[0].toObject()["background mined"].toArray(),
      score_data.score_data.second);
    return score_data;
  }
}  // namespace analyzer
