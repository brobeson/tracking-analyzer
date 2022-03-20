#include "qtag.h"
#include <QApplication>
#include <gsl/gsl_util>

namespace analyzer::gui
{
  namespace
  {
    struct tag_colors
    {
      QColor foreground;
      QColor background;
      QColor border;
      // Clang-tidy altera-struct-pack-align warns about inefficient alignment
      // of this structure. The check recommends 64-bit alignment.
      // NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers)
    } __attribute__((aligned(64)));

    // The algorithms in make_colors_dark_theme() and make_colors_light_theme()
    // come from examining Github's issue label CSS. I had to make a few
    // modifications because the CSS color structure behaves slightly
    // differently from QColor.
    auto make_colors_dark_theme(const QColor& requested_color)
    {
      static constexpr qreal lightness_threshold {0.6};
      static constexpr qreal background_alpha {0.18};
      static constexpr qreal border_alpha {0.3};
      static constexpr int maximum_component {255};
      QColor background {
        requested_color.red(),
        requested_color.green(),
        requested_color.blue(),
        gsl::narrow_cast<int>(maximum_component * background_alpha)};
      const auto perceived_lightness {requested_color.redF() * 0.2126
                                      + requested_color.greenF() * 0.7152
                                      + requested_color.blueF() * 0.0722};
      const auto lightness_switch {std::clamp(
        (perceived_lightness - lightness_threshold) * -1000, 0.0, 1.0)};
      const auto lighten_by {(lightness_threshold - perceived_lightness) * 100.0
                             * lightness_switch};
      static constexpr qreal one_percent {0.01};
      QColor foreground {QColor::fromHslF(requested_color.hueF(),
                                          requested_color.hslSaturationF(),
                                          requested_color.lightnessF()
                                            + lighten_by * one_percent)};
      QColor border {QColor::fromHslF(requested_color.hueF(),
                                      requested_color.saturationF(),
                                      requested_color.lightnessF(),
                                      border_alpha)};
      return tag_colors {foreground, background, border};
    }

    auto make_colors_light_theme(const QColor& requested_color)
    {
      static constexpr qreal lightness_threshold {0.453};
      static constexpr qreal border_threshold {0.96};
      const auto perceived_lightness {requested_color.redF() * 0.2126
                                      + requested_color.greenF() * 0.7152
                                      + requested_color.blueF() * 0.0722};
      const qreal border_alpha {
        std::clamp((perceived_lightness - border_threshold) * 100.0, 0.0, 1.0)};
      const auto lightness_switch {std::clamp(
        (perceived_lightness - lightness_threshold) * -1000, 0.0, 1.0)};
      return tag_colors {
        QColor::fromHslF(0.0, 0.0, lightness_switch * 1.0),
        requested_color,
        QColor::fromHslF(
          requested_color.hueF(),
          requested_color.saturationF(),
          // 0.25 is Github's CSS without context. Since I don't know how they
          // came up with this number, or what it represents, I can't create a
          // meaningful identifier.
          // NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers)
          requested_color.lightnessF() - 0.25,
          border_alpha)};
    }

    auto to_string(const QColor& color)
    {
      return QString {"rgba("} + QString::number(color.red()) + ','
             + QString::number(color.green()) + ','
             + QString::number(color.blue()) + ','
             + QString::number(color.alpha()) + ')';
    }

    auto make_color_styles(const tag_colors& colors)
    {
      return QString {"background-color: "} + to_string(colors.background)
             + QString {"; border: 1px solid "} + to_string(colors.border)
             + QString {"; color: "} + to_string(colors.foreground) + ';';
    }

    auto create_tag_label_stylesheet(const QColor& text_color)
    {
      const auto lightness {
        QApplication::palette().color(QPalette::Window).lightnessF()};
      static constexpr qreal mid_lightness {0.5};
      return QString {"QLabel{border-radius: 10px; "}
             + make_color_styles(lightness < mid_lightness
                                   ? make_colors_dark_theme(text_color)
                                   : make_colors_light_theme(text_color))
             + '}';
    }
  }  // namespace

  qtag::qtag(const QString& text, const QColor& text_color, QWidget* parent):
    QLabel {text, parent}
  {
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    setMargin(4);
    setStyleSheet(create_tag_label_stylesheet(text_color));
  }

  qtag::qtag(const QString& text, QWidget* parent):
    qtag {text, QApplication::palette().color(QPalette::WindowText), parent}
  {
  }
}  // namespace analyzer::gui
