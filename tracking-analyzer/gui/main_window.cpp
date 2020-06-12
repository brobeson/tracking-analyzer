#include "main_window.h"
#include "ui_main_window.h"

namespace analyzer::gui
{
  main_window::main_window(QWidget* parent):
    QMainWindow(parent), ui(new Ui::main_window)
  {
    ui->setupUi(this);
  }

  main_window::~main_window() { delete ui; }

  void main_window::add_graph(QtCharts::QScatterSeries* graph)
  {
    ui->graphicsView->chart()->addSeries(graph);
    ui->graphicsView->chart()->createDefaultAxes();
  }
}  // namespace analyzer::gui
