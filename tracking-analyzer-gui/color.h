#ifndef ANALYZER_COLOR_CYCLER_H
#define ANALYZER_COLOR_CYCLER_H

#include <QColor>
#include <vector>

namespace analyzer::gui
{
  class color_map
  {
  public:
    using size_type = std::vector<QColor>::size_type;

    explicit color_map(size_type n);

    [[nodiscard]] auto operator[](size_type i) const -> QColor;
    [[nodiscard]] auto size() const noexcept -> size_type;

  private:
    std::vector<QColor> m_colors;
  };

  void print(const QColor& color);
}  // namespace analyzer::gui

#endif
