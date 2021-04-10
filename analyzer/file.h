#ifndef ANALYZER_FILE_H
#define ANALYZER_FILE_H

#include <QDir>
#include <QString>

namespace analyzer
{
  [[nodiscard]] QDir make_absolute_path(const QString& path) noexcept;
  [[nodiscard]] bool path_exists(const QString& path) noexcept;
}  // namespace analyzer

#endif