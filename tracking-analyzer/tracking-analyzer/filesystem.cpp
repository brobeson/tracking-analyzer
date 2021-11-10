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
}  // namespace analyzer
