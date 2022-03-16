#ifndef ANALYZER_TRACKING_RESULTS_H
#define ANALYZER_TRACKING_RESULTS_H

#include "tracking-analyzer/bounding_box.h"
#include <QList>
#include <QString>
#include <array>
#include <utility>
#include <vector>

namespace analyzer
{
  struct tracking_results final
  {
    QString sequence_name;
    std::vector<analyzer::bounding_box> bounding_boxes;
  };

  struct tracker_results final
  {
    explicit tracker_results(const std::string& tracker_name);
    void add_sequence_results(const tracking_results& results);

    // NOLINTNEXTLINE(misc-non-private-member-variables-in-classes)
    std::vector<tracking_results> sequence_results;

    // NOLINTNEXTLINE(misc-non-private-member-variables-in-classes)
    std::string m_name;
  };

  struct results_database final
  {
    std::vector<tracker_results> m_trackers;
  };

  [[nodiscard]] auto get_sequence_results(const tracker_results& db,
                                          const std::string& sequence_name)
    -> tracking_results;

  [[nodiscard]] auto get_tracker_results(const results_database& db,
                                         const std::string& tracker_name)
    -> tracker_results;

  [[nodiscard]] auto get_trackers_in_database(const results_database& db)
    -> QStringList;

  [[nodiscard]] auto load_tracking_results_for_sequence(const QString& path)
    -> tracking_results;

  [[nodiscard]] auto load_tracking_results_directory(const QString& path)
    -> results_database;
}  // namespace analyzer

#endif
