#ifndef ANALYZER_FILESYSTEM_H
#define ANALYZER_FILESYSTEM_H

#include <QString>

namespace analyzer
{
  auto make_absolute_path(const QString& path) -> QString;
}  // namespace analyzer

#endif
