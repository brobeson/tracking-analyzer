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

    void update_sequence_combobox(QComboBox& combobox,
                                  const QStringList& sequence_names)
    {
      combobox.clear();
      combobox.addItems(sequence_names);
      combobox.setCurrentIndex(-1);  // Ensure no item is selected.
    }

    void clear_display(const Ui::main_window& display)
    {
      display.offset_graph->chart()->removeAllSeries();
      display.overlap_graph->chart()->removeAllSeries();
      display.frame_display->setPixmap(QPixmap {});
    }
  }  // namespace

  main_window::main_window(QWidget* parent):
    QMainWindow(parent), ui(new Ui::main_window)
  {
    ui->setupUi(this);
    analyzer::gui::setup_offset_chart(ui->offset_graph->chart());
    analyzer::gui::setup_overlap_chart(ui->overlap_graph->chart());
    connect(ui->dataset_path,
            &QLineEdit::textEdited,
            this,
            &analyzer::gui::main_window::check_dataset_path);
    connect(ui->dataset_path,
            &QLineEdit::editingFinished,
            this,
            &analyzer::gui::main_window::load_dataset);
    connect(ui->sequence,
            QOverload<int>::of(&QComboBox::currentIndexChanged),
            this,
            &analyzer::gui::main_window::sequence_changed);
    check_dataset_path(ui->dataset_path->text());
    load_dataset();
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

  void main_window::check_dataset_path(QString path_text) const
  {
    try
    {
      const auto absolute_path {analyzer::make_absolute_path(path_text)};
      if (std::filesystem::is_directory(absolute_path.toStdString()))
      {
        ui->dataset_path->setStyleSheet("");
      }
      else
      {
        ui->dataset_path->setStyleSheet("QLineEdit#dataset_path{color:red}");
      }
    }
    catch (...)
    {
      ui->dataset_path->setStyleSheet("");
    }
  }

  void main_window::load_dataset()
  {
    const auto new_dataset {analyzer::load_dataset(ui->dataset_path->text())};
    if (!new_dataset.root_path().isEmpty())
    {
      m_dataset = new_dataset;
      analyzer::gui::update_sequence_combobox(
        *(ui->sequence), analyzer::sequence_names(m_dataset.sequences()));
      ui->sequence_count->setText(
        QString::number(m_dataset.sequences().length()) + " sequences.");
    }
  }

  void main_window::sequence_changed(const int index) const
  {
    analyzer::gui::clear_display(*ui);
    if (index >= 0)
    {
    }
  }
}  // namespace analyzer::gui
