#include "dataset.h"
#include <QDir>
#include <filesystem>

namespace analyzer
{
  namespace
  {
    QStringList read_sequence_names(const QString& dataset_path)
    {
      const QDir directory {dataset_path};
      return directory.entryList(QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name);
    }

    QVector<analyzer::sequence> read_sequences(const QString& dataset_path)
    {
      const QDir directory {dataset_path};
      const auto sequence_names {
        directory.entryList(QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name)};
      QVector<analyzer::sequence> sequences;
      sequences.reserve(sequence_names.size());
      for (const auto& name : sequence_names)
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

    QStringList make_sequence_frame_paths(const QString& sequence_path)
    {
      QDir directory {sequence_path};
      directory.cd("img");
      auto frame_paths {
        directory.entryList({"*.jpg"}, QDir::Files, QDir::Name)};
      for (auto& frame_path : frame_paths)
      {
        frame_path.prepend('/').prepend(directory.absolutePath());
      }
      return frame_paths;
    }
  }  // namespace

  invalid_sequence::invalid_sequence(const QString& name,
                                     const QString& path,
                                     const std::string& what):
    std::runtime_error {what}, m_name {name}, m_path {path}
  {
  }

  QString invalid_sequence::name() const { return m_name; }
  QString invalid_sequence::path() const { return m_path; }

  sequence::sequence(const QString& name, const QString& path):
    m_name {name},
    m_root_path {path},
    m_frame_paths {analyzer::make_sequence_frame_paths(path)}
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

  QString sequence::name() const { return m_name; }
  QStringList sequence::frame_paths() const { return m_frame_paths; }

  dataset::dataset(const QString& root_path,
                   const QVector<sequence>& sequences):
    m_root_directory {root_path}, m_sequences {sequences}
  {
  }

  const QVector<analyzer::sequence>& dataset::sequences() const noexcept
  {
    return m_sequences;
  }

  // const QStringList& dataset::sequence_names() const noexcept
  // {
  //   return m_sequence_names;
  // }

  const QString& dataset::root_path() const noexcept
  {
    return m_root_directory;
  }

  QString make_absolute_path(const QString& path)
  {
    if (path.front() == '~')
    {
      return path.right(path.length() - 1).prepend(QDir::homePath());
    }
    return path;
  }

  analyzer::dataset load_dataset(const QString& path)
  {
    const auto dataset_path {analyzer::make_absolute_path(path)};
    const auto sequences {analyzer::read_sequence_names(dataset_path)};
    return analyzer::dataset {dataset_path,
                              analyzer::read_sequences(dataset_path)};
  }

  QStringList sequence_names(const QVector<analyzer::sequence>& sequences)
  {
    QStringList names;
    names.reserve(sequences.length());
    for (const auto& sequence : sequences)
    {
      names.push_back(sequence.name());
    }
    return names;
  }
}  // namespace analyzer