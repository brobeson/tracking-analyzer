#include "tracking-analyzer/training_metadata.h"
#include "tracking-analyzer/filesystem.h"
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <algorithm>
#include <gsl/gsl_util>

namespace analyzer
{
  namespace
  {
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

    auto read_json_data(const QString& filepath)
    {
      QFile file {filepath};
      if (!file.open(QIODevice::ReadOnly))
      {
        throw std::runtime_error {"cannot open file"};
      }
      return QJsonDocument::fromJson(file.readAll()).object();
    }

    auto get_chart_range(const score_list& scores)
    {
      if (scores.empty())
      {
        // TODO Change this to something better.
        return range {0.0f, 0.0f};
      }
      const auto minimum_x {
        std::min_element(
          std::begin(scores),
          std::end(scores),
          [](const QPointF& a, const QPointF& b) { return a.x() < b.x(); })
          ->x()};
      const auto maximum_x {
        std::max_element(
          std::begin(scores),
          std::end(scores),
          [](const QPointF& a, const QPointF& b) { return a.x() < b.x(); })
          ->x()};
      const auto minimum_y {
        std::min_element(
          std::begin(scores),
          std::end(scores),
          [](const QPointF& a, const QPointF& b) { return a.y() < b.y(); })
          ->y()};
      const auto maximum_y {
        std::max_element(
          std::begin(scores),
          std::end(scores),
          [](const QPointF& a, const QPointF& b) { return a.y() < b.y(); })
          ->y()};
      return range {std::min(minimum_x, minimum_y),
                    std::max(maximum_x, maximum_y)};
    }

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

    auto parse_training_batch(const QJsonObject& json)
    {
      training_batch batch;
      fill_training_scores(json["background candidates"].toArray(),
                           batch.background_candidates);
      fill_training_scores(json["background mined"].toArray(),
                           batch.background_mined);
      fill_training_scores(json["target candidates"].toArray(),
                           batch.target_candidates);
      batch.background_threshold
        = gsl::narrow_cast<float>(json["thresholds"].toArray()[0].toDouble());
      batch.target_threshold
        = gsl::narrow_cast<float>(json["thresholds"].toArray()[1].toDouble());
      return batch;
    }

    auto parse_training_update(const QJsonArray& json)
    {
      training_update update;
      try
      {
        update.reserve(gsl::narrow<training_update::size_type>(json.size()));
      }
      catch (const gsl::narrowing_error&)
      {
        throw analyzer::parse_error {
          "Cannot reserve " + std::to_string(json.size())
          + " training batches in analyzer::parse_training_update()."};
      }
      std::transform(std::begin(json),
                     std::end(json),
                     std::back_insert_iterator {update},
                     [](const QJsonValue& json_value) {
                       return parse_training_batch(json_value.toObject());
                     });
      return update;
    }

    auto parse_all_training_updates(const QJsonObject& json)
    {
      update_list updates;
      try
      {
        updates.reserve(gsl::narrow<update_list::size_type>(json.size()));
      }
      catch (const gsl::narrowing_error&)
      {
        throw analyzer::parse_error {
          "Cannot reserve " + std::to_string(json.size())
          + " training updates in analyzer::parse_training_updates()."};
      }
      std::transform(std::begin(json),
                     std::end(json),
                     std::back_insert_iterator {updates},
                     [](const QJsonValue& json_value) {
                       return parse_training_update(json_value.toArray());
                     });
      return updates;
    }
  }  // namespace

  auto get_chart_range(const training_batch& batch) -> range
  {
    const auto bg_candidate_range {
      get_chart_range(batch.background_candidates)};
    const auto bg_mined_range {get_chart_range(batch.background_mined)};
    const auto tg_candidate_range {get_chart_range(batch.target_candidates)};
    return range {std::min(std::min(std::min(std::min(bg_candidate_range.first,
                                                      bg_mined_range.first),
                                             tg_candidate_range.first),
                                    batch.background_threshold),
                           batch.target_threshold),
                  std::max(std::max(std::max(std::max(bg_candidate_range.second,
                                                      bg_mined_range.second),
                                             tg_candidate_range.second),
                                    batch.background_threshold),
                           batch.target_threshold)};
  }

  auto load_training_scores(const QString& path) -> training_scores
  {
    if (path.isEmpty())
    {
      throw std::invalid_argument {
        "The path to the training score data is empty."};
    }
    const auto absolute_path {make_absolute_path(path)};
    const auto json_data {read_json_data(absolute_path)};
    const auto score_json {json_data["data"].toObject()};
    training_scores score_data {json_data["sequence"].toString(),
                                json_data["dataset"].toString(),
                                parse_training_score_update_frames(score_json),
                                parse_all_training_updates(score_json)};
    return score_data;
  }
}  // namespace analyzer
