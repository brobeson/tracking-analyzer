#include "tracking-analyzer/dataset.h"
#include "tracking-analyzer/filesystem.h"
#include <QDir>
#include <filesystem>
#include <fstream>

namespace analyzer
{
  namespace
  {
    auto read_sequence_names(const QString& dataset_path)
    {
      const QDir directory {dataset_path};
      return directory.entryList(QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name);
    }

    auto read_sequences(const QString& dataset_path)
    {
      const auto names {read_sequence_names(dataset_path)};
      QVector<analyzer::sequence> sequences;
      sequences.reserve(names.size());
      for (const auto& name : names)
      {
        try
        {
          sequences.push_back(
            analyzer::sequence {name, dataset_path + '/' + name});
        }
        catch (...)
        {
          // Do nothing for now. Maybe try to report it in the future.
        }
      }
      return sequences;
    }

    auto make_sequence_frame_paths(const QString& sequence_path)
    {
      QDir directory {sequence_path};
      if (!directory.cd("img"))
      {
        throw std::invalid_argument {
          sequence_path.toStdString()
          + " does not contain the img subdirectory."};
      }
      auto frame_paths {
        directory.entryList({"*.jpg"}, QDir::Files, QDir::Name)};
      for (auto& frame_path : frame_paths)
      {
        frame_path.prepend('/').prepend(directory.absolutePath());
      }
      return frame_paths;
    }

    auto read_ground_truth_boxes(const std::string& path)
    {
      std::ifstream s {path + "/groundtruth_rect.txt"};
      return analyzer::read_bounding_boxes(s);
    }
  }  // namespace

  invalid_sequence::invalid_sequence(const QString& name,
                                     const QString& path,
                                     const std::string& what):
    std::runtime_error {what}, m_name {name}, m_path {path}
  {
  }

  auto invalid_sequence::name() const -> QString { return m_name; }
  auto invalid_sequence::path() const -> QString { return m_path; }

  sequence::sequence(const QString& name, const QString& path):
    m_name {name},
    m_root_path {path},
    m_frame_paths {analyzer::make_sequence_frame_paths(path)},
    m_target_boxes {analyzer::read_ground_truth_boxes(path.toStdString())}
  {
    if (m_root_path.isEmpty())
    {
      throw analyzer::invalid_sequence {
        m_name, m_root_path, "A sequence cannot have an empty root path."};
    }
    if (!QDir {m_root_path}.exists())
    {
      throw analyzer::invalid_sequence {
        m_name,
        m_root_path,
        "The sequence path " + m_root_path.toStdString() + " does not exist."};
    }
    if (m_frame_paths.isEmpty())
    {
      throw analyzer::invalid_sequence {
        m_name,
        m_root_path,
        "The sequence " + m_name.toStdString() + " at path "
          + m_root_path.toStdString() + " does not have frame images."};
    }
  }

  auto sequence::name() const -> QString { return m_name; }
  auto sequence::frame_paths() const -> QStringList { return m_frame_paths; }
  auto sequence::path() const -> QString { return m_root_path; }
  auto sequence::target_boxes() const -> analyzer::bounding_box_list
  {
    return m_target_boxes;
  }

  dataset::dataset(const QString& root_path,
                   const QVector<sequence>& sequences):
    m_root_directory {root_path}, m_sequences {sequences}
  {
  }

  auto dataset::sequences() const noexcept -> const QVector<analyzer::sequence>&
  {
    return m_sequences;
  }

  auto dataset::root_path() const noexcept -> const QString&
  {
    return m_root_directory;
  }

  auto load_dataset(const QString& path) -> analyzer::dataset
  {
    const auto dataset_path {analyzer::make_absolute_path(path)};
    return analyzer::dataset {dataset_path,
                              analyzer::read_sequences(dataset_path)};
  }

  auto sequence_names(const QVector<analyzer::sequence>& sequences)
    -> QStringList
  {
    QStringList names;
    std::transform(std::begin(sequences),
                   std::end(sequences),
                   std::back_insert_iterator {names},
                   [](const analyzer::sequence& s) { return s.name(); });
    return names;
  }
}  // namespace analyzer