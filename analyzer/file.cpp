#include "file.h"

namespace analyzer
{
  auto make_absolute_path(const QString& path) noexcept -> QDir
  {
    if (path.at(0) == '~')
    {
      return QDir {path.right(path.length() - 1).prepend(QDir::homePath())};
    }
    return QDir {path};
  }

  auto path_exists(const QString& path) noexcept -> bool
  {
    return make_absolute_path(path).exists();
  }
}  // namespace analyzer