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

  void main_window::set_offset_data(QtCharts::QScatterSeries* offset_data)
  {
    ui->offset_graph->chart()->addSeries(offset_data);
    ui->offset_graph->chart()->createDefaultAxes();
  }

  void main_window::set_overlap_data(QtCharts::QScatterSeries* overlap_data)
  {
    ui->overlap_graph->chart()->addSeries(overlap_data);
    ui->overlap_graph->chart()->createDefaultAxes();
  }

  void main_window::set_image(const QImage& image)
  {
    ui->label->setPixmap(QPixmap::fromImage(image));
  }
}  // namespace analyzer::gui
