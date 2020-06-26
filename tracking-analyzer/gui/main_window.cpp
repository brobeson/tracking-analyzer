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

    auto make_overlap_y_axis() -> QtCharts::QValueAxis*
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

    auto make_offset_y_axis(const qreal maximum) -> QtCharts::QValueAxis*
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

    auto make_x_axis(const qreal maximum) -> QtCharts::QValueAxis*
    {
      QtCharts::QValueAxis* axis {nullptr};
      try
      {
        axis = new QtCharts::QValueAxis();
        axis->setRange(0.0, maximum);
        axis->setTickAnchor(0.0);
        axis->setTickInterval(10.0);  // NOLINT
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
      if (axis == nullptr)
      {
        chart->createDefaultAxes();
      }
      else
      {
        chart->addAxis(axis, Qt::AlignLeft);
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
      display.frame_number->setText("");
      display.frame_slider->setValue(0);
      display.sequence_path->setText("");
    }

    void set_current_frame(const Ui::main_window& display, int number)
    {
      display.frame_number->setText(QString::number(number));
      display.frame_slider->setValue(number);
    }

    void load_and_display_frame(const QString& frame_path, QLabel& display)
    {
      QImageReader reader {frame_path};
      display.setPixmap(QPixmap::fromImage(reader.read()));
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
    if (x_axis == nullptr || y_axis == nullptr)
    {
      ui->offset_graph->chart()->createDefaultAxes();
    }
    else
    {
      ui->offset_graph->chart()->addAxis(x_axis, Qt::AlignBottom);
      ui->offset_graph->chart()->addAxis(y_axis, Qt::AlignLeft);
    }
  }

  void main_window::set_overlap_data(QtCharts::QScatterSeries* overlap_data)
  {
    ui->overlap_graph->chart()->addSeries(overlap_data);
    const auto axis {
      analyzer::gui::make_x_axis(overlap_data->pointsVector().size())};
    if (axis == nullptr)
    {
      ui->overlap_graph->chart()->createDefaultAxes();
    }
    else
    {
      ui->overlap_graph->chart()->addAxis(axis, Qt::AlignBottom);
    }
  }

  void main_window::check_dataset_path(const QString& path_text) const
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
      analyzer::gui::set_current_frame(*ui, 1);
      ui->sequence_path->setText(m_dataset.sequences()[index].path());
      analyzer::gui::load_and_display_frame(
        m_dataset.sequences()[index].frame_paths()[1], *(ui->frame_display));
      ui->frame_number->setEnabled(true);
      ui->frame_slider->setEnabled(true);
      ui->frame_slider->setMinimum(1);
      ui->frame_slider->setMaximum(
        m_dataset.sequences()[index].frame_paths().length());
    }
  }
}  // namespace analyzer::gui
