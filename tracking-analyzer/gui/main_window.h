#ifndef ANALYZER_GUI_MAIN_WINDOW_H
#define ANALYZER_GUI_MAIN_WINDOW_H

#include <QLineSeries>
#include <QMainWindow>

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
    ~main_window();

    void add_graph(QtCharts::QLineSeries* graph);

  private:
    Ui::main_window* ui;
  };
}  // namespace analyzer::gui

#endif  // ANALYZER_GUI_MAIN_WINDOW_H