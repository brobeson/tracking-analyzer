#include "color.h"
#include <cmath>
#include <gsl/gsl_util>
#include <iostream>

namespace analyzer::gui
{
  namespace
  {
    auto make_color_array(const color_map::size_type n)
    {
      if (n == 0)
      {
        return std::vector<QColor> {};
      }
      constexpr auto maximum_hue {359};
      const auto stride {maximum_hue / gsl::narrow_cast<int>(n)};
      auto hue {0};
      std::vector<QColor> colors(n);
      std::generate(std::begin(colors), std::end(colors), [&hue, stride]() {
        constexpr auto maximum_sl {255};
        auto c {QColor::fromHsv(hue, maximum_sl, maximum_sl).toRgb()};
        hue += stride;
        return c;
      });
      return colors;
    }
  }  // namespace

  color_map::color_map(const size_type n): m_colors {make_color_array(n)} {}

  auto color_map::operator[](const size_type i) const -> QColor
  {
    return m_colors.at(i);
  }

  auto color_map::size() const noexcept -> size_type { return m_colors.size(); }
}  // namespace analyzer::gui
