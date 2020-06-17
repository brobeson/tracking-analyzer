#include "main_window.h"
#include "ui_main_window.h"
#include <QValueAxis>
#include <algorithm>
#include <filesystem>

namespace analyzer::gui
{
  namespace
  {
    void setup_offset_chart(QtCharts::QChart* chart)
    {
      chart->setTitle("Center Offset (pixels)");
    }

    QtCharts::QValueAxis* make_overlap_y_axis()
    {
      QtCharts::QValueAxis* axis {nullptr};
      try
      {
        axis = new QtCharts::QValueAxis();
        axis->setRange(0.0, 1.0);
        axis->setTickAnchor(0.0);
        axis->setTitleText("Overlap Ratio");
        return axis;
      }
      catch (...)
      {
        delete axis;
        return nullptr;
      }
    }

    QtCharts::QValueAxis* make_offset_y_axis(const qreal maximum)
    {
      QtCharts::QValueAxis* axis {nullptr};
      try
      {
        axis = new QtCharts::QValueAxis();
        axis->setRange(0.0, maximum);
        axis->setTickAnchor(0.0);
        axis->setTitleText("Center Offset (pixels)");
        return axis;
      }
      catch (...)
      {
        delete axis;
        return nullptr;
      }
    }

    QtCharts::QValueAxis* make_x_axis(const qreal maximum)
    {
      QtCharts::QValueAxis* axis {nullptr};
      try
      {
        axis = new QtCharts::QValueAxis();
        axis->setRange(0.0, maximum);
        axis->setTickAnchor(0.0);
        axis->setTickInterval(10.0);
        axis->setTickType(QtCharts::QValueAxis::TicksDynamic);
        axis->setLabelFormat("%i");
        axis->setTitleText("Frame Number");
        return axis;
      }
      catch (...)
      {
        delete axis;
        return nullptr;
      }
    }

    void setup_overlap_chart(QtCharts::QChart* chart)
    {
      chart->setTitle("Overlap Ratio");
      const auto axis {analyzer::gui::make_overlap_y_axis()};
      if (axis)
      {
        chart->addAxis(axis, Qt::AlignLeft);
      }
      else
      {
        chart->createDefaultAxes();
      }
    }

    class path_validator final: public QValidator
    {
      virtual ~path_validator() = default;

      QValidator::State validate(QString& /*intput*/,
                                 int& /*position*/) const final
      {
        return QValidator::Intermediate;
      }
    };
  }  // namespace

  main_window::main_window(QWidget* parent):
    QMainWindow(parent), ui(new Ui::main_window)
  {
    ui->setupUi(this);
    analyzer::gui::setup_offset_chart(ui->offset_graph->chart());
    analyzer::gui::setup_overlap_chart(ui->overlap_graph->chart());
    ui->dataset_path->setValidator(new analyzer::gui::path_validator);
  }

  main_window::~main_window() { delete ui; }

  void main_window::set_offset_data(QtCharts::QScatterSeries* offset_data)
  {
    ui->offset_graph->chart()->addSeries(offset_data);
    const auto offset_vector {offset_data->pointsVector()};
    const auto x_axis {analyzer::gui::make_x_axis(offset_vector.size())};
    const auto y_axis {analyzer::gui::make_offset_y_axis(
      std::max_element(
        std::begin(offset_vector),
        std::end(offset_vector),
        [](const auto& a, const auto& b) { return a.y() < b.y(); })
        ->y())};
    if (x_axis && y_axis)
    {
      ui->offset_graph->chart()->addAxis(x_axis, Qt::AlignBottom);
      ui->offset_graph->chart()->addAxis(y_axis, Qt::AlignLeft);
    }
    else
    {
      ui->offset_graph->chart()->createDefaultAxes();
    }
  }

  void main_window::set_overlap_data(QtCharts::QScatterSeries* overlap_data)
  {
    ui->overlap_graph->chart()->addSeries(overlap_data);
    const auto axis {
      analyzer::gui::make_x_axis(overlap_data->pointsVector().size())};
    if (axis)
    {
      ui->overlap_graph->chart()->addAxis(axis, Qt::AlignBottom);
    }
    else
    {
      ui->overlap_graph->chart()->createDefaultAxes();
    }
  }

  void main_window::set_image(const QImage& image)
  {
    ui->frame_display->setPixmap(QPixmap::fromImage(image));
  }

  // void main_window::change_dataset_path(const QString& path_text) const
  // {
  //   try
  //   {
  //     if (std::filesystem::is_directory(path_text.toStdString()))
  //     {
  //       ui->dataset_path->
  //     }
  //   }
  //   catch (...)
  //   {
  //     return false;
  //   }
  // }
}  // namespace analyzer::gui
