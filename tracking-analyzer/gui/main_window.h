#ifndef ANALYZER_GUI_MAIN_WINDOW_H
#define ANALYZER_GUI_MAIN_WINDOW_H

#include <QImage>
#include <QMainWindow>
#include <QScatterSeries>

namespace analyzer::gui
{
  namespace Ui
  {
    class main_window;
  }

  class main_window: public QMainWindow
  {
    Q_OBJECT

  public:
    explicit main_window(QWidget* parent = nullptr);
    main_window(const main_window&) = delete;
    main_window& operator=(const main_window&) = delete;
    ~main_window();

    void set_offset_data(QtCharts::QScatterSeries* data);
    void set_overlap_data(QtCharts::QScatterSeries* data);
    void set_image(const QImage& image);

  private:
    Ui::main_window* ui;
  };
}  // namespace analyzer::gui

#endif  // ANALYZER_GUI_MAIN_WINDOW_H
