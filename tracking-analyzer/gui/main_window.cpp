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

    void make_training_series(QChart& chart,
                              const score_list& scores,
                              const QString& name,
                              const qreal point_size)
    {
      auto* const series {new QtCharts::QScatterSeries};
      series->append(scores);
      series->setName(name);
      // NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers)
      series->setMarkerSize(point_size);
      series->setPen(QPen {});
      chart.addSeries(series);
    }

    auto calculate_point_size(const QSlider& slider)
    {
      constexpr auto MINIMUM_SIZE {5.0f};
      constexpr auto MAXIMUM_SIZE {15.0f};
      const auto t {static_cast<float>(slider.value())
                    / static_cast<float>(slider.maximum())};
      return ((1.0f - t) * MINIMUM_SIZE + t * MAXIMUM_SIZE);
    }

    void add_decision_boundary(QChart& chart, const range graph_bounds)
    {
      auto* const line {new QLineSeries};
      line->append(graph_bounds.first, graph_bounds.first);
      line->append(graph_bounds.second, graph_bounds.second);
      line->setName("Decision Boundary");
      chart.addSeries(line);
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
    connect(ui->update_frame_number,
            qOverload<int>(&QSpinBox::valueChanged),
            this,
            &analyzer::gui::main_window::change_update_frame);
    connect(ui->point_size_slider,
            &QSlider::sliderMoved,
            this,
            &analyzer::gui::main_window::change_point_size);
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

  void main_window::set_training_score_data(const training_iteration& iteration)
  {
    ui->overlap_graph->chart()->removeAllSeries();
    auto& chart {*(ui->overlap_graph->chart())};
    const auto range {get_chart_range(iteration)};
    add_decision_boundary(chart, range);
    const auto point_size {calculate_point_size(*ui->point_size_slider)};
    make_training_series(chart,
                         iteration.background_candidates,
                         "Background Training Candidates",
                         point_size);
    make_training_series(
      chart, iteration.background_mined, "Background Mined", point_size);
    make_training_series(chart,
                         iteration.target_candidates,
                         "Target Training Candidates",
                         point_size);
    ui->overlap_graph->chart()->createDefaultAxes();
    auto* axis {
      ui->overlap_graph->chart()->axes(Qt::Orientation::Horizontal)[0]};
    axis->setTitleText("Background Scores");
    axis->setRange(range.first, range.second);
    axis = ui->overlap_graph->chart()->axes(Qt::Orientation::Vertical)[0];
    axis->setTitleText("Target Scores");
    axis->setRange(range.first, range.second);
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

  void main_window::change_update_frame(const int update_frame_index) const
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
      ui->update_frame_slider->setValue(update_frame_index);
      ui->update_frame_number->setValue(update_frame_index);
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
    // TODO Change this a gsl::narrow_cast<int>.
    const auto maximum {static_cast<int>(m_training_data.update_frames.size())};
    ui->sequence->setCurrentText(m_training_data.sequence_name);
    ui->update_frame_slider->setEnabled(true);
    ui->update_frame_slider->setMinimum(0);
    ui->update_frame_slider->setMaximum(maximum);
    ui->update_frame_slider->setValue(0);
    ui->update_frame_number->setEnabled(true);
    ui->update_frame_number->setMaximum(maximum);
    ui->update_frame_number->setSuffix(" of " + QString::number(maximum));
    set_training_score_data(m_training_data.iteration_scores);
  }

  void main_window::change_point_size(const int /*index*/) const
  {
    const auto size {calculate_point_size(*ui->point_size_slider)};
    for (auto* const series : ui->overlap_graph->chart()->series())
    {
      if (series->type() == QAbstractSeries::SeriesType::SeriesTypeScatter)
      {
        dynamic_cast<QScatterSeries*>(series)->setMarkerSize(size);
      }
    }
  }
}  // namespace analyzer::gui
