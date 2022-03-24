#include "tracking-analyzer/tracking_results.h"
#include "tracking-analyzer/filesystem.h"
#include <QDir>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <fstream>

namespace analyzer
{
  using namespace std::literals::string_literals;

  sequence_results::sequence_results(
    const std::string& sequence_name,
    const bounding_box_list& target_bounding_boxes):
    m_name {sequence_name}, m_target_boxes {target_bounding_boxes}
  {
  }

  auto sequence_results::name() const -> std::string { return m_name; }

  auto sequence_results::bounding_boxes() const noexcept
    -> const bounding_box_list&
  {
    return m_target_boxes;
  }

  auto sequence_results::bounding_boxes() noexcept -> bounding_box_list&
  {
    return m_target_boxes;
  }

  auto sequence_results::operator[](bounding_box_list::size_type i) const
    -> const bounding_box&
  {
    return m_target_boxes.at(i);
  }

  auto sequence_results::operator[](bounding_box_list::size_type i)
    -> bounding_box&
  {
    return m_target_boxes.at(i);
  }

  auto size(const sequence_results& sequence) noexcept
    -> bounding_box_list::size_type
  {
    return sequence.bounding_boxes().size();
  }

  auto begin(const sequence_results& sequence)
    -> bounding_box_list::const_iterator
  {
    return std::begin(sequence.bounding_boxes());
  }

  auto end(const sequence_results& sequence)
    -> bounding_box_list::const_iterator
  {
    return std::end(sequence.bounding_boxes());
  }

  tracker_results::tracker_results(const std::string& tracker_name,
                                   const sequence_list& tracked_sequences):
    m_name {tracker_name}, m_sequences {tracked_sequences}
  {
  }

  auto tracker_results::name() const -> std::string { return m_name; }

  auto tracker_results::sequences() const noexcept -> const sequence_list&
  {
    return m_sequences;
  }

  auto tracker_results::sequences() noexcept -> sequence_list&
  {
    return m_sequences;
  }

  auto tracker_results::operator[](const size_type i) const
    -> const sequence_results&
  {
    return m_sequences.at(i);
  }

  auto tracker_results::operator[](const size_type i) -> sequence_results&
  {
    return m_sequences.at(i);
  }

  auto tracker_results::operator[](const std::string& sequence_name) const
    -> const sequence_results&
  {
    const auto i {
      std::find_if(std::begin(m_sequences),
                   std::end(m_sequences),
                   [&sequence_name](const sequence_results& results) {
                     return results.name() == sequence_name;
                   })};
    if (i == std::end(m_sequences))
    {
      throw invalid_sequence {
        sequence_name,
        "",
        // NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers)
        "sequence not found in "s
          + m_name
          // NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers)
          + " tracking results"s};
    }
    return *i;
  }

  auto tracker_results::operator[](const std::string& sequence_name)
    -> sequence_results&
  {
    const auto i {
      std::find_if(std::begin(m_sequences),
                   std::end(m_sequences),
                   [&sequence_name](const sequence_results& results) {
                     return results.name() == sequence_name;
                   })};
    if (i == std::end(m_sequences))
    {
      throw invalid_sequence {
        sequence_name,
        "",
        // NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers)
        "sequence not found in "s
          + m_name
          // NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers)
          + " tracking results"s};
    }
    return *i;
  }

  auto size(const tracker_results& tracker) noexcept
    -> tracker_results::size_type
  {
    return tracker.sequences().size();
  }

  auto begin(const tracker_results& tracker)
    -> tracker_results::sequence_list::const_iterator
  {
    return std::begin(tracker.sequences());
  }

  auto end(const tracker_results& tracker)
    -> tracker_results::sequence_list::const_iterator
  {
    return std::end(tracker.sequences());
  }

  auto results_database::trackers() const noexcept -> const tracker_list&
  {
    return m_trackers;
  }

  auto results_database::trackers() noexcept -> tracker_list&
  {
    return m_trackers;
  }

  auto results_database::operator[](const std::string& tracker_name) const
    -> const tracker_results&
  {
    const auto i {std::find_if(std::begin(m_trackers),
                               std::end(m_trackers),
                               [&tracker_name](const tracker_results& results) {
                                 return results.name() == tracker_name;
                               })};
    if (i == std::end(m_trackers))
    {
      throw invalid_tracker {
        tracker_name,
        // NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers)
        tracker_name + " not found in results database"s};
    }
    return *i;
  }

  auto results_database::operator[](const std::string& tracker_name)
    -> tracker_results&
  {
    const auto i {std::find_if(std::begin(m_trackers),
                               std::end(m_trackers),
                               [&tracker_name](const tracker_results& results) {
                                 return results.name() == tracker_name;
                               })};
    if (i == std::end(m_trackers))
    {
      throw invalid_tracker {
        tracker_name,
        // NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers)
        tracker_name + " not found in results database"s};
    }
    return *i;
  }

  auto size(const results_database& db) noexcept -> results_database::size_type
  {
    return db.trackers().size();
  }

  auto begin(const results_database& db) noexcept
    -> results_database::tracker_list::const_iterator
  {
    return std::begin(db.trackers());
  }

  auto end(const results_database& db) noexcept
    -> results_database::tracker_list::const_iterator
  {
    return std::end(db.trackers());
  }

  auto list_all_trackers(const results_database& db) -> std::vector<std::string>
  {
    std::vector<std::string> trackers;
    std::transform(begin(db),
                   end(db),
                   std::back_inserter(trackers),
                   [](const tracker_results& r) { return r.name(); });
    return trackers;
  }

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

    [[nodiscard]] auto load_tracking_results_for_sequence(const QString& path)
    {
      const auto lines {read_result_lines(make_absolute_path(path))};
      return sequence_results {
        analyzer::basename(path).replace(".txt", "").toStdString(),
        parse_lines(lines)};
    }

    [[nodiscard]] auto
    load_tracking_results_for_tracker(const QString& path,
                                      const QString& tracker_name)
    {
      const auto sequences {get_sequence_file_paths(path + '/' + tracker_name)};
      tracker_results r {tracker_name.toStdString(), {}};
      std::transform(std::begin(sequences),
                     std::end(sequences),
                     std::back_inserter(r.sequences()),
                     [&path, &tracker_name](const QString& sequence) {
                       return load_tracking_results_for_sequence(
                         path + '/' + tracker_name + '/' + sequence);
                     });
      return r;
    }
  }  // namespace

  auto load_tracking_results_directory(const std::string& path)
    -> results_database
  {
    const auto root_path {QString::fromStdString(path)};
    const auto trackers {analyzer::get_subdirectories(path)};
    results_database db;
    std::transform(std::begin(trackers),
                   std::end(trackers),
                   std::back_inserter(db.trackers()),
                   [&root_path](const QString& tracker) {
                     return load_tracking_results_for_tracker(root_path,
                                                              tracker);
                   });
    return db;
  }

  void push_front(results_database& db, const tracker_results& tracker)
  {
    db.trackers().insert(std::begin(db.trackers()), tracker);
  }
}  // namespace analyzer
