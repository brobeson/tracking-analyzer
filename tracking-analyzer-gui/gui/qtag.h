#ifndef ANALYZER_GUI_QTAG_H
#define ANALYZER_GUI_QTAG_H

#include <QColor>
#include <QLabel>

namespace analyzer::gui
{
  class qtag final: public QLabel
  {
  public:
    qtag(const QString& text, QWidget* parent);
    qtag(const QString& text, const QColor& text_color, QWidget* parent);
  };
}  // namespace analyzer::gui

#endif
