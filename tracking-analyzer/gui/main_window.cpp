#include "main_window.h"
#include "tracking_results.h"
#include "ui_main_window.h"
#include <QScatterSeries>
#include <QValueAxis>
#include <algorithm>
#include <filesystem>

namespace analyzer::gui
{
  namespace
  {
    // NOLINTNEXTLINE(clang-diagnostic-unused-parameter)
    //     void set_tick_anchor(QtCharts::QValueAxis& axis, const qreal anchor)
    //     {
    //       // The tick anchor property was added in Qt 5.12. If the Qt version
    //       is
    //       // earlier than that, no-op.
    // #if QT_VERSION >= 0x051200
    //       axis.setTickAnchor(anchor);
    // #endif
    //     }

    // NOLINTNEXTLINE(clang-diagnostic-unused-parameter)
    //     void set_tick_interval(QtCharts::QValueAxis& axis, const qreal
    //     interval)
    //     {
    // #if QT_VERSION >= 0x051200
    //       axis.setTickInterval(interval);
    // #endif
    //     }

    // #if QT_VERSION >= 0x051200
    //     using tick_type = QtCharts::QValueAxis::TickType;
    // #else
    //     using tick_type = int;
    // #endif

    // NOLINTNEXTLINE(clang-diagnostic-unused-parameter)
    //     void set_tick_type(QtCharts::QValueAxis& axis,
    //                        //
    //                        NOLINTNEXTLINE(clang-diagnostic-unused-parameter)
    //                        const tick_type type)
    //     {
    // #if QT_VERSION >= 0x051200
    //       axis.setTickType(type);
    // #endif
    //     }

    void setup_offset_chart(QtCharts::QChart* chart)
    {
      chart->setTitle("Center Offset (pixels)");
    }

    // auto make_overlap_y_axis() -> QtCharts::QValueAxis*
    // {
    //   QtCharts::QValueAxis* axis {nullptr};
    //   try
    //   {
    //     axis = new QtCharts::QValueAxis();
    //     axis->setRange(0.0, 1.0);
    //     // axis->setTickAnchor(*axis, 0.0);
    //     axis->setTitleText("Overlap Ratio");
    //     return axis;
    //   }
    //   catch (...)
    //   {
    //     delete axis;
    //     return nullptr;
    //   }
    // }

    auto make_offset_y_axis(const qreal maximum) -> QtCharts::QValueAxis*
    {
      QtCharts::QValueAxis* axis {nullptr};
      try
      {
        axis = new QtCharts::QValueAxis();
        axis->setRange(0.0, maximum);
        // axis->setTickAnchor(*axis, 0.0);
        axis->setTitleText("Center Offset (pixels)");
        return axis;
      }
      catch (...)
      {
        delete axis;
        return nullptr;
      }
    }

    auto make_x_axis(const qreal /*maximum*/) -> QtCharts::QValueAxis*
    {
      QtCharts::QValueAxis* axis {nullptr};
      try
      {
        axis = new QtCharts::QValueAxis();
        // axis->setRange(0.0, maximum);
        // axis->setTickAnchor(*axis, 0.0);
        // axis->setTickInterval(*axis, 10.0);
        // axis->setTickType(*axis, QtCharts::QValueAxis::TicksDynamic);
        axis->setLabelFormat("%i");
        axis->setTitleText("Background Scores");
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
      chart->setTitle("Training Scores");
      // auto* const axis {analyzer::gui::make_overlap_y_axis()};
      // if (axis == nullptr)
      // {
      //   chart->createDefaultAxes();
      // }
      // else
      // {
      //   chart->addAxis(axis, Qt::AlignLeft);
      // }
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
      display.frame_number->setValue(0);
      display.frame_slider->setValue(0);
      display.sequence_path->setText("");
    }

    void set_current_frame(const Ui::main_window& display, int number)
    {
      display.frame_number->setValue(number);
      display.frame_slider->setValue(number);
    }

    void draw_bounding_box(QImage& image, const analyzer::bounding_box box)
    {
      QPainter painter {&image};
      painter.setBrush(Qt::NoBrush);
      painter.setPen(Qt::red);
      painter.drawRect(QRectF {box.x, box.y, box.width, box.height});
    }

    auto load_frame(const QString& frame_path)
    {
      QImage frame {frame_path};
      if (frame.format() != QImage::Format_RGB32)
      {
        frame = frame.convertToFormat(QImage::Format_RGB32);
      }
      return frame;
    }

    void load_and_display_frame(const QString& frame_path,
                                QLabel& display,
                                const analyzer::bounding_box box)
    {
      auto frame {load_frame(frame_path)};
      draw_bounding_box(frame, box);
      display.setPixmap(QPixmap::fromImage(frame));
    }

    auto get_chart_bounds(const QList<QPointF>& data)
    {
      const auto minimum_x {
        std::min_element(
          std::begin(data),
          std::end(data),
          [](const QPointF& a, const QPointF& b) { return a.x() < b.x(); })
          ->x()};
      const auto maximum_x {
        std::max_element(
          std::begin(data),
          std::end(data),
          [](const QPointF& a, const QPointF& b) { return a.x() < b.x(); })
          ->x()};
      const auto minimum_y {
        std::min_element(
          std::begin(data),
          std::end(data),
          [](const QPointF& a, const QPointF& b) { return a.y() < b.y(); })
          ->y()};
      const auto maximum_y {
        std::max_element(
          std::begin(data),
          std::end(data),
          [](const QPointF& a, const QPointF& b) { return a.y() < b.y(); })
          ->y()};
      return QPointF {std::min(minimum_x, minimum_y),
                      std::max(maximum_x, maximum_y)};
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
    connect(ui->frame_slider,
            &QSlider::sliderMoved,
            this,
            &analyzer::gui::main_window::change_frame);
    connect(ui->frame_number,
            qOverload<int>(&QSpinBox::valueChanged),
            this,
            &analyzer::gui::main_window::change_frame);
    connect(ui->results_path,
            &QLineEdit::editingFinished,
            this,
            &analyzer::gui::main_window::load_tracking_data);
    connect(ui->update_frame_slider,
            &QSlider::sliderMoved,
            this,
            &analyzer::gui::main_window::change_update_frame);
    check_dataset_path(ui->dataset_path->text());
    load_dataset();
    // load_tracking_results(ui->results_path->text());

    // sequence_changed() is called by Qt during initialization. Reset the
    // frame slider and line edit enabled properties.
    ui->frame_number->setEnabled(false);
    ui->frame_slider->setEnabled(false);
  }

  main_window::~main_window() { delete ui; }

  void main_window::set_offset_data(QtCharts::QScatterSeries* offset_data)
  {
    ui->offset_graph->chart()->addSeries(offset_data);
    const auto offset_vector {offset_data->pointsVector()};
    auto* const x_axis {analyzer::gui::make_x_axis(offset_vector.size())};
    auto* const y_axis {analyzer::gui::make_offset_y_axis(
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
    ui->overlap_graph->chart()->removeAllSeries();
    overlap_data->setName("Background Training Candidates");
    ui->overlap_graph->chart()->addSeries(overlap_data);
    // auto* const axis {
    //   analyzer::gui::make_x_axis(overlap_data->pointsVector().size())};
    // if (axis == nullptr)
    // {
    //   ui->overlap_graph->chart()->createDefaultAxes();
    // }
    // else
    // {
    //   ui->overlap_graph->chart()->addAxis(axis, Qt::AlignBottom);
    // }
    const auto range {get_chart_bounds(overlap_data->points())};
    ui->overlap_graph->chart()->createDefaultAxes();
    auto axis {
      ui->overlap_graph->chart()->axes(Qt::Orientation::Horizontal)[0]};
    axis->setTitleText("Background Scores");
    axis->setRange(range.x(), range.y());
    axis = ui->overlap_graph->chart()->axes(Qt::Orientation::Vertical)[0];
    axis->setTitleText("Target Scores");
    axis->setRange(range.x(), range.y());
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

  void main_window::sequence_changed(const int index)
  {
    analyzer::gui::clear_display(*ui);
    if (index >= 0)
    {
      m_sequence_index = index;
      analyzer::gui::set_current_frame(*ui, 0);
      ui->sequence_path->setText(m_dataset.sequences()[index].path());
      analyzer::gui::load_and_display_frame(
        m_dataset.sequences()[index].frame_paths()[1],
        *(ui->frame_display),
        m_dataset.sequences()[index].target_boxes()[1]);
      const auto maximum_frame {
        m_dataset.sequences()[index].frame_paths().length() - 1};
      ui->frame_number->setEnabled(true);
      ui->frame_number->setSuffix(" of " + QString::number(maximum_frame));
      ui->frame_number->setMaximum(maximum_frame);
      ui->frame_slider->setEnabled(true);
      ui->frame_slider->setMinimum(0);
      ui->frame_slider->setMaximum(maximum_frame);
    }
  }

  // void main_window::load_tracking_results(const QString& path)  // NOLINT
  // {
  //   const auto new_results {analyzer::load_tracking_results(path)};
  // }

  void main_window::change_frame(const int frame_index) const
  {
    if (frame_index < 0)
    {
      return;
    }
    analyzer::gui::set_current_frame(*ui, frame_index);
    analyzer::gui::load_and_display_frame(
      m_dataset.sequences()[m_sequence_index].frame_paths()[frame_index],
      *(ui->frame_display),
      m_dataset.sequences()[m_sequence_index]
        .target_boxes()[static_cast<analyzer::bounding_box_list::size_type>(
          frame_index)]);
  }

  void main_window::change_update_frame(int update_frame_index) const
  {
    if (update_frame_index < 0)
    {
      return;
    }
    try
    {
      const auto frame {m_training_data.update_frames.at(
        static_cast<std::vector<int>::size_type>(update_frame_index))};
      if (ui->frame_slider->maximum() < frame)
      {
        return;
      }
      change_frame(frame);
    }
    catch (...)
    {
      return;
    }
  }

  void main_window::load_tracking_data()
  {
    m_training_data = analyzer::load_training_scores(ui->results_path->text());
    ui->sequence->setCurrentText(m_training_data.sequence_name);
    ui->update_frame_slider->setEnabled(true);
    ui->update_frame_slider->setMinimum(0);
    ui->update_frame_slider->setMaximum(
      // TODO Change this a gsl::narrow_cast<int>.
      static_cast<int>(m_training_data.update_frames.size()));
    ui->update_frame_slider->setValue(0);
    const auto series {new QtCharts::QScatterSeries};
    series->append(m_training_data.score_data);
    set_overlap_data(series);
  }
}  // namespace analyzer::gui
