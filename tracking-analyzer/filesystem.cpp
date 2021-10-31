#include "filesystem.h"
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
    return path.right(path.length() - path.lastIndexOf('/') - 1);
  }
}  // namespace analyzer
