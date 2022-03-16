#ifndef ANALYZER_FILESYSTEM_H
#define ANALYZER_FILESYSTEM_H

#include <QString>

namespace analyzer
{
  auto make_absolute_path(const QString& path) -> QString;
  auto basename(const QString& path) -> QString;
  auto get_subdirectories(const QString& path) -> QStringList;
}  // namespace analyzer

#endif
