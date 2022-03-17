#include "tracking-analyzer/filesystem.h"
#include <QDir>

namespace analyzer
{
  auto make_absolute_path(const QString& path) -> QString
  {
    if (path.at(0) == '~')
    {
      return path.right(path.length() - 1).prepend(QDir::homePath());
    }
    return path;
  }

  auto basename(const QString& path) -> QString
  {
    if (path.isEmpty())
    {
      return path;
    }
    auto base {path.right(path.length() - path.lastIndexOf('/', -2) - 1)};
    if (*base.crbegin() == '/')
    {
      base.remove(base.length() - 1, 1);
    }
    return base;
  }

  auto get_subdirectories(const QString& path) -> QStringList
  {
    const QDir directory {path};
    return directory.entryList(QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name);
  }

  auto read_json_data(const QString& filepath) -> QJsonDocument
  {
    QFile file {filepath};
    if (!file.open(QIODevice::ReadOnly))
    {
      throw std::runtime_error {"cannot open file"};
    }
    return QJsonDocument::fromJson(file.readAll());
  }

  auto get_sequence_file_paths(const QString& path, const QString& file_suffix)
    -> QStringList
  {
    const QDir directory {path};
    auto sequences {directory.entryList(
      QDir::Files | QDir::NoSymLinks | QDir::NoDotAndDotDot, QDir::Name)};
    sequences.erase(std::remove_if(std::begin(sequences),
                                   std::end(sequences),
                                   [&file_suffix](const QString& sequence) {
                                     return !sequence.endsWith(file_suffix);
                                   }),
                    sequences.end());
    return sequences;
  }
}  // namespace analyzer
