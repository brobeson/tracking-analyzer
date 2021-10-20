#include "tracking_results.h"
#include "dataset.h"
#include <QDir>
#include <QJsonDocument>
#include <QJsonObject>

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
    training_scores score_data {json_data["sequence"].toString(),
                                json_data["dataset"].toString()};
    return score_data;
  }
}  // namespace analyzer
