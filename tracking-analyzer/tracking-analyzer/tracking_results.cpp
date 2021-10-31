#include "tracking-analyzer/tracking_results.h"
#include "tracking-analyzer/filesystem.h"
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
  }  // namespace

  auto load_tracking_results(const QString& path)
    -> std::vector<tracking_results>
  {
    const auto absolute_path {make_absolute_path(path)};
    const auto sequences {analyzer::read_sequence_names(absolute_path)};
    return std::vector<tracking_results> {};
  }
}  // namespace analyzer
