#include "tracking-analyzer/tracking_results.h"
#include "tracking-analyzer/filesystem.h"
#include <QDir>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <fstream>

namespace analyzer
{
  namespace
  {
    [[nodiscard]] auto read_result_lines(const QString& filepath)
    {
      QStringList lines;
      std::string one_line;
      std::ifstream file {filepath.toStdString()};
      while (file)
      {
        file >> one_line;
        if (!one_line.empty())
        {
          lines.append(QString::fromStdString(one_line));
        }
      }
      return lines;
    }

    [[nodiscard]] auto parse_line(const QString& line)
    {
      const auto numbers {line.split(",")};
      return bounding_box {numbers.at(0).toFloat(),
                           numbers.at(1).toFloat(),
                           numbers.at(2).toFloat(),
                           numbers.at(3).toFloat()};
    }

    [[nodiscard]] auto parse_lines(const QStringList& lines)
    {
      std::vector<bounding_box> boxes;
      std::transform(std::begin(lines),
                     std::end(lines),
                     std::back_insert_iterator {boxes},
                     &parse_line);
      return boxes;
    }

    [[nodiscard]] auto get_sequence_file_paths(const QString& path)
    {
      const QDir directory {path};
      auto sequences {directory.entryList(
        QDir::Files | QDir::NoSymLinks | QDir::NoDotAndDotDot, QDir::Name)};
      sequences.erase(std::remove_if(std::begin(sequences),
                                     std::end(sequences),
                                     [](const QString& sequence) {
                                       return !sequence.endsWith(".txt");
                                     }),
                      sequences.end());
      return sequences;
    }

    [[nodiscard]] auto
    load_tracking_results_for_tracker(const QString& path,
                                      const QString& tracker_name)
    {
      const auto sequences {get_sequence_file_paths(path + '/' + tracker_name)};
      tracker_results r {tracker_name.toStdString()};
      std::transform(std::begin(sequences),
                     std::end(sequences),
                     std::back_inserter(r.sequence_results),
                     [&path, &tracker_name](const QString& sequence) {
                       return load_tracking_results_for_sequence(
                         path + '/' + tracker_name + '/' + sequence);
                     });
      return r;
    }
  }  // namespace

  auto load_tracking_results_for_sequence(const QString& path)
    -> tracking_results
  {
    const auto lines {read_result_lines(make_absolute_path(path))};
    return tracking_results {analyzer::basename(path).replace(".txt", ""),
                             parse_lines(lines)};
  }

  tracker_results::tracker_results(const std::string& tracker_name):
    m_name {tracker_name}
  {
  }

  auto load_tracking_results_directory(const QString& path) -> results_database
  {
    const auto results_path {analyzer::make_absolute_path(path)};
    const auto trackers {analyzer::get_subdirectories(results_path)};
    results_database db;
    std::transform(std::begin(trackers),
                   std::end(trackers),
                   std::back_inserter(db.m_trackers),
                   [&results_path](const auto& tracker) {
                     return load_tracking_results_for_tracker(results_path,
                                                              tracker);
                   });
    return db;
  }

  auto get_trackers_in_database(const results_database& db) -> QStringList
  {
    QStringList trackers;
    std::transform(std::cbegin(db.m_trackers),
                   std::cend(db.m_trackers),
                   std::back_inserter(trackers),
                   [](const tracker_results& r) {
                     return QString::fromStdString(r.m_name);
                   });
    return trackers;
  }

  auto get_tracker_results(const results_database& db,
                           const std::string& tracker_name) -> tracker_results
  {
    const auto i {
      std::find_if(std::begin(db.m_trackers),
                   std::end(db.m_trackers),
                   [&tracker_name](const tracker_results& candidate) {
                     return candidate.m_name == tracker_name;
                   })};
    if (i == db.m_trackers.end())
    {
      throw std::runtime_error {"Tracker not found in results database."};
    }
    return *i;
  }

  auto get_sequence_results(const tracker_results& db,
                            const std::string& sequence_name)
    -> tracking_results
  {
    const auto i {std::find_if(
      std::begin(db.sequence_results),
      std::end(db.sequence_results),
      [&sequence_name](const tracking_results& candidate) {
        return candidate.sequence_name.toStdString() == sequence_name;
      })};
    if (i == std::end(db.sequence_results))
    {
      throw std::runtime_error {"Sequence not found in results database."};
    }
    return *i;
  }
}  // namespace analyzer
