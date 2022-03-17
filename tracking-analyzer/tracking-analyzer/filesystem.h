#ifndef ANALYZER_FILESYSTEM_H
#define ANALYZER_FILESYSTEM_H

#include <QJsonDocument>
#include <QStringList>

namespace analyzer
{
  [[nodiscard]] auto make_absolute_path(const QString& path) -> QString;
  [[nodiscard]] auto basename(const QString& path) -> QString;
  [[nodiscard]] auto get_subdirectories(const QString& path) -> QStringList;
  [[nodiscard]] auto read_json_data(const QString& filepath) -> QJsonDocument;
  [[nodiscard]] auto get_sequence_file_paths(const QString& path,
                                             const QString& file_suffix)
    -> QStringList;
}  // namespace analyzer

#endif
