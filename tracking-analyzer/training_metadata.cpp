#include "training_metadata.h"
#include "filesystem.h"
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <algorithm>

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
  }  // namespace

  auto get_chart_range(const training_iteration& iteration) -> range
  {
    const auto bg_candidate_range {
      get_chart_range(iteration.background_candidates)};
    const auto bg_mined_range {get_chart_range(iteration.background_mined)};
    const auto tg_candidate_range {
      get_chart_range(iteration.target_candidates)};
    return range {
      std::min(std::min(bg_candidate_range.first, bg_mined_range.first),
               tg_candidate_range.first),
      std::max(std::max(bg_candidate_range.second, bg_mined_range.second),
               tg_candidate_range.second)};
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
                         score_data.iteration_scores.background_candidates);
    fill_training_scores(
      score_json["0"].toArray()[0].toObject()["background mined"].toArray(),
      score_data.iteration_scores.background_mined);
    fill_training_scores(
      score_json["0"].toArray()[0].toObject()["target candidates"].toArray(),
      score_data.iteration_scores.target_candidates);
    return score_data;
  }
}  // namespace analyzer
