#include "main_window.h"
#include "filesystem.h"
#include "tracking_results.h"
#include "ui_main_window.h"
#include <QComboBox>
#include <QFileDialog>
#include <QScatterSeries>
#include <QSpinBox>
#include <QValueAxis>
#include <algorithm>
#include <filesystem>

namespace analyzer::gui
{
  namespace settings_keys
  {
    constexpr auto last_loaded_tracking_data {"recent/tracking_data_path"};
    constexpr auto last_loaded_dataset {"recent/dataset_path"};
  }  // namespace settings_keys

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

    // void setup_offset_chart(QtCharts::QChart* chart)
    // {
    //   chart->setTitle("Center Offset (pixels)");
    // }

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

    // auto make_offset_y_axis(const qreal maximum) -> QtCharts::QValueAxis*
    // {
    //   QtCharts::QValueAxis* axis {nullptr};
    //   try
    //   {
    //     axis = new QtCharts::QValueAxis();
    //     axis->setRange(0.0, maximum);
    //     // axis->setTickAnchor(*axis, 0.0);
    //     axis->setTitleText("Center Offset (pixels)");
    //     return axis;
    //   }
    //   catch (...)
    //   {
    //     delete axis;
    //     return nullptr;
    //   }
    // }

    // auto make_x_axis(const qreal /*maximum*/) -> QtCharts::QValueAxis*
    // {
    //   QtCharts::QValueAxis* axis {nullptr};
    //   try
    //   {
    //     axis = new QtCharts::QValueAxis();
    //     // axis->setRange(0.0, maximum);
    //     // axis->setTickAnchor(*axis, 0.0);
    //     // axis->setTickInterval(*axis, 10.0);
    //     // axis->setTickType(*axis, QtCharts::QValueAxis::TicksDynamic);
    //     axis->setLabelFormat("%i");
    //     axis->setTitleText("Background Scores");
    //     return axis;
    //   }
    //   catch (...)
    //   {
    //     delete axis;
    //     return nullptr;
    //   }
    // }

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
      display.overlap_graph->chart()->removeAllSeries();
      display.frame_display->setPixmap(QPixmap {});
      display.frame_number->setValue(0);
      display.frame_slider->setValue(0);
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

    void add_decision_boundary(QChart& chart, const range graph_bounds)
    {
      auto* const line {new QLineSeries};
      line->append(graph_bounds.first, graph_bounds.first);
      line->append(graph_bounds.second, graph_bounds.second);
      line->setName("Decision Boundary");
      chart.addSeries(line);
    }

    auto find_data_series(const QChart& chart, const QString& name)
      -> QAbstractSeries*
    {
      const auto all_series {chart.series()};
      const auto iterator {
        std::find_if(std::begin(all_series),
                     std::end(all_series),
                     [&name](const QAbstractSeries* const series) {
                       return series->name() == name;
                     })};
      if (iterator == std::end(all_series))
      {
        return nullptr;
      }
      return *iterator;
    }

    void add_score_thresholds(QChart& chart,
                              const range graph_bounds,
                              const float background_threshold,
                              const float target_threshold)
    {
      const auto color {
        dynamic_cast<QScatterSeries*>(
          find_data_series(chart, "Background Training Candidates"))
          ->color()};
      auto* line {new QLineSeries};
      line->append(background_threshold, graph_bounds.first);
      line->append(background_threshold, graph_bounds.second);
      line->setName("Cluster Thresholds");
      line->setColor(color);
      chart.addSeries(line);
      line = new QLineSeries;
      line->append(graph_bounds.first, target_threshold);
      line->append(graph_bounds.second, target_threshold);
      line->setColor(color);
      chart.addSeries(line);
      chart.legend()->markers().back()->setVisible(false);
    }

    void toggle_series(const Ui::main_window& ui,
                       const QString& series_name,
                       const bool series_visible)
    {
      auto* const series {
        find_data_series(*ui.overlap_graph->chart(), series_name)};
      if (series != nullptr)
      {
        series->setVisible(series_visible);
      }
    }

    auto get_tracking_data_filepath(main_window* parent,
                                    const QSettings& settings)
    {
      return QFileDialog::getOpenFileName(
        parent,
        "Load Tracking Data",
        settings
          .value(settings_keys::last_loaded_tracking_data, QDir::homePath())
          .toString(),
        "JSON (*.json)");
    }

    void reset_spinbox(QSpinBox& spinbox, const int maximum)
    {
      spinbox.setEnabled(true);
      spinbox.setMaximum(maximum);
      spinbox.setValue(1);
      spinbox.setSuffix(" of " + QString::number(maximum));
    }

    template <typename T>
    auto get_toolbar_widget(const QToolBar& toolbar, QAction* action)
    {
      return dynamic_cast<T*>(toolbar.widgetForAction(action));
    }
  }  // namespace

  main_window::main_window(QWidget* parent):
    QMainWindow(parent),
    ui(new Ui::main_window),
    m_update_spinbox {new QSpinBox},
    m_batch_spinbox {new QSpinBox},
    m_sequence_combobox {new QComboBox},
    m_point_size_spinbox {new QSpinBox}
  {
    ui->setupUi(this);
    setWindowTitle("");
    add_graph_controls_to_toolbar();
    analyzer::gui::setup_overlap_chart(ui->overlap_graph->chart());
    connect(ui->action_load_dataset,
            &QAction::triggered,
            this,
            qOverload<>(&analyzer::gui::main_window::load_dataset));
    connect(ui->frame_slider,
            &QSlider::sliderMoved,
            this,
            &analyzer::gui::main_window::change_frame);
    connect(ui->frame_number,
            qOverload<int>(&QSpinBox::valueChanged),
            this,
            &analyzer::gui::main_window::change_frame);
    connect(ui->bg_candidates_checkbox,
            &QCheckBox::clicked,
            this,
            &analyzer::gui::main_window::toggle_bg_candidate_plot);
    connect(ui->bg_mined_checkbox,
            &QCheckBox::clicked,
            this,
            &analyzer::gui::main_window::toggle_bg_mined_plot);
    connect(ui->tg_candidates_checkbox,
            &QCheckBox::clicked,
            this,
            &analyzer::gui::main_window::toggle_tg_candidate_plot);
    connect(ui->action_save_graph,
            &QAction::triggered,
            this,
            &main_window::save_graph);
    connect(ui->action_open_tracking_data,
            &QAction::triggered,
            this,
            &analyzer::gui::main_window::load_tracking_data);
    if (settings.contains(settings_keys::last_loaded_dataset))
    {
      load_dataset(
        settings.value(settings_keys::last_loaded_dataset).toString());
    }

    // sequence_changed() is called by Qt during initialization. Reset the
    // frame slider and line edit enabled properties.
    ui->frame_number->setEnabled(false);
    ui->frame_slider->setEnabled(false);
  }

  main_window::~main_window() { delete ui; }

  // void main_window::set_offset_data(QtCharts::QScatterSeries* offset_data)
  // {
  //   ui->offset_graph->chart()->addSeries(offset_data);
  //   const auto offset_vector {offset_data->pointsVector()};
  //   auto* const x_axis {analyzer::gui::make_x_axis(offset_vector.size())};
  //   auto* const y_axis {analyzer::gui::make_offset_y_axis(
  //     std::max_element(
  //       std::begin(offset_vector),
  //       std::end(offset_vector),
  //       [](const auto& a, const auto& b) { return a.y() < b.y(); })
  //       ->y())};
  //   if (x_axis == nullptr || y_axis == nullptr)
  //   {
  //     ui->offset_graph->chart()->createDefaultAxes();
  //   }
  //   else
  //   {
  //     ui->offset_graph->chart()->addAxis(x_axis, Qt::AlignBottom);
  //     ui->offset_graph->chart()->addAxis(y_axis, Qt::AlignLeft);
  //   }
  // }

  void main_window::set_training_score_data(const training_batch& batch)
  {
    ui->overlap_graph->chart()->removeAllSeries();
    auto& chart {*ui->overlap_graph->chart()};
    auto& legend {*chart.legend()};
    legend.setAlignment(Qt::AlignRight);
    const auto range {get_chart_range(batch)};
    add_decision_boundary(chart, range);
    const auto point_size {m_point_size_spinbox->value()};
    make_training_series(chart,
                         batch.background_candidates,
                         "Background Training Candidates",
                         point_size);
    make_training_series(
      chart, batch.background_mined, "Background Mined", point_size);
    make_training_series(
      chart, batch.target_candidates, "Target Training Candidates", point_size);
    add_score_thresholds(
      chart, range, batch.background_threshold, batch.target_threshold);
    ui->overlap_graph->chart()->createDefaultAxes();
    auto* axis {
      ui->overlap_graph->chart()->axes(Qt::Orientation::Horizontal)[0]};
    axis->setTitleText("Background Scores");
    axis->setRange(range.first, range.second);
    axis = ui->overlap_graph->chart()->axes(Qt::Orientation::Vertical)[0];
    axis->setTitleText("Target Scores");
    axis->setRange(range.first, range.second);
  }

  void main_window::load_dataset()
  {
    const auto dataset_path {QFileDialog::getExistingDirectory(
      this,
      "Load Dataset",
      settings.value(settings_keys::last_loaded_dataset, QDir::homePath())
        .toString())};
    if (!dataset_path.isEmpty())
    {
      load_dataset(dataset_path);
    }
  }

  void main_window::load_dataset(const QString& dataset_path)
  {
    const auto new_dataset {analyzer::load_dataset(dataset_path)};
    if (!new_dataset.root_path().isEmpty())
    {
      m_dataset = new_dataset;
      analyzer::gui::update_sequence_combobox(
        *m_sequence_combobox, analyzer::sequence_names(m_dataset.sequences()));
      settings.setValue(settings_keys::last_loaded_dataset, dataset_path);
      settings.sync();
    }
  }

  void main_window::sequence_changed(const int index)
  {
    analyzer::gui::clear_display(*ui);
    if (index >= 0)
    {
      m_sequence_index = index;
      analyzer::gui::set_current_frame(*ui, 0);
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

  void main_window::change_update(const int update_number)
  {
    const auto update_index {
      static_cast<update_list::size_type>(update_number - 1)};
    const auto frame {m_training_data.update_frames.at(update_index)};
    change_frame(frame);
    m_current_training.current_update = update_index;
    reset_spinbox(
      *m_batch_spinbox,
      static_cast<int>(m_training_data.updates[update_index].size()));
    change_training_batch(1);
  }

  void main_window::load_tracking_data(const bool /*unused*/)
  {
    const auto filepath {get_tracking_data_filepath(this, settings)};
    if (filepath.isEmpty())
    {
      return;
    }
    m_training_data = analyzer::load_training_scores(filepath);
    m_sequence_combobox->setCurrentText(m_training_data.sequence_name);
    reset_spinbox(*m_update_spinbox,
                  static_cast<int>(m_training_data.update_frames.size()));
    change_update(1);
    m_point_size_spinbox->setEnabled(true);
    settings.setValue(settings_keys::last_loaded_tracking_data, filepath);
    settings.sync();
    setWindowTitle(basename(filepath));
  }

  void main_window::change_point_size(const int size) const
  {
    for (auto* const series : ui->overlap_graph->chart()->series())
    {
      if (series->type() == QAbstractSeries::SeriesType::SeriesTypeScatter)
      {
        dynamic_cast<QScatterSeries*>(series)->setMarkerSize(size);
      }
    }
  }

  void main_window::toggle_bg_candidate_plot(const bool checked) const
  {
    toggle_series(*ui, "Background Training Candidates", checked);
  }

  void main_window::toggle_bg_mined_plot(const bool checked) const
  {
    toggle_series(*ui, "Background Mined", checked);
  }

  void main_window::toggle_tg_candidate_plot(const bool checked) const
  {
    toggle_series(*ui, "Target Training Candidates", checked);
  }

  void main_window::save_graph(const bool /*unused*/)
  {
    const auto suggestion {QDir::homePath() + "/"
                           + m_training_data.sequence_name
                           + "_frame1_update1_scores.png"};
    const auto filepath {QFileDialog::getSaveFileName(
      this, "Save Plot", suggestion, "PNG (*.png)")};
    if (!filepath.isEmpty())
    {
      const auto image {ui->overlap_graph->grab()};
      image.save(filepath, "PNG");
    }
  }

  void main_window::change_training_batch(const int batch_number)
  {
    m_current_training.current_batch
      = static_cast<training_update::size_type>(batch_number - 1);
    set_training_score_data(
      m_training_data.updates.at(m_current_training.current_update)
        .at(m_current_training.current_batch));
  }

  void main_window::add_graph_controls_to_toolbar()
  {
    m_update_spinbox->setMinimum(1);
    m_update_spinbox->setPrefix("Update ");
    m_update_spinbox->setEnabled(false);
    m_update_spinbox->setToolTip("Iterate through the training updates.");
    connect(m_update_spinbox,
            qOverload<int>(&QSpinBox::valueChanged),
            this,
            &analyzer::gui::main_window::change_update);
    ui->toolbar->insertWidget(ui->action_load_dataset, m_update_spinbox);

    m_batch_spinbox->setMinimum(1);
    m_batch_spinbox->setPrefix("Batch ");
    m_batch_spinbox->setEnabled(false);
    m_batch_spinbox->setToolTip("Iterate through the training batches.");
    connect(m_batch_spinbox,
            qOverload<int>(&QSpinBox::valueChanged),
            this,
            &analyzer::gui::main_window::change_training_batch);
    ui->toolbar->insertWidget(ui->action_load_dataset, m_batch_spinbox);

    m_point_size_spinbox->setPrefix("Point Size ");
    // NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers)
    m_point_size_spinbox->setMinimum(5);
    // NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers)
    m_point_size_spinbox->setMaximum(15);
    // NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers)
    m_point_size_spinbox->setValue(15);
    m_point_size_spinbox->setEnabled(false);
    m_point_size_spinbox->setToolTip(
      "Change the size of the data points in the graph.");
    connect(m_point_size_spinbox,
            qOverload<int>(&QSpinBox::valueChanged),
            this,
            &analyzer::gui::main_window::change_point_size);
    ui->toolbar->insertWidget(ui->action_load_dataset, m_point_size_spinbox);

    auto* const widget {new QWidget};
    widget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    ui->toolbar->insertWidget(ui->action_load_dataset, widget);

    m_sequence_combobox->setSizeAdjustPolicy(QComboBox::AdjustToContents);
    m_sequence_combobox->setToolTip(
      "Change the sequence displayed in the sequence view.");
    connect(m_sequence_combobox,
            QOverload<int>::of(&QComboBox::currentIndexChanged),
            this,
            &analyzer::gui::main_window::sequence_changed);
    ui->toolbar->insertWidget(ui->action_load_dataset, m_sequence_combobox);
  }
}  // namespace analyzer::gui
