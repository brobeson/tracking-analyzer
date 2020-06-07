#include "gui/main_window.h"
#include <QApplication>

int main(int argc, char* argv[])
{
  QApplication application {argc, argv};
  analyzer::gui::main_window window;
  window.show();

  auto line {new QtCharts::QLineSeries};
  line->append(0, 6);
  line->append(2, 4);
  window.add_graph(line);
  return application.exec();
}