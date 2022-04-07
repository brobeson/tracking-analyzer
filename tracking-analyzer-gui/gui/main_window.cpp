#include "main_window.h"
#include "application.h"
#include "qtag.h"
#include "ui_main_window.h"
#include <QComboBox>
#include <QDir>
#include <QFileDialog>
#include <QLabel>
#include <QMenu>
#include <QPainter>
#include <QToolButton>

namespace analyzer::gui
{
  namespace
  {
    constexpr std::chrono::milliseconds status_bar_message_timeout {5000};

    void reinitialize_combobox(QComboBox& combobox,
                               const analyzer::name_list& new_entries)
    {
      combobox.clear();
      for (const auto& entry : new_entries)
      {
        combobox.addItem(QString::fromStdString(entry));
      }
      combobox.setCurrentIndex(-1);  // Ensure no item is selected.
    }

    void clear_display(const Ui::main_window& display)
    {
      display.frame_display->setPixmap(QPixmap {});
      display.frame_display->setText("No Sequence Loaded");
      display.frame_spinbox->setValue(0);
      display.frame_slider->setValue(0);
    }

    void synchronize_frame_controls(const Ui::main_window& display, int number)
    {
      display.frame_spinbox->setValue(number);
      display.frame_slider->setValue(number);
    }

    auto get_tracking_path_to_draw(const bounding_box_list& boxes)
    {
      QVector<QPointF> points;
      std::transform(
        std::begin(boxes),
        std::end(boxes),
        std::back_inserter(points),
        [](const bounding_box& box) { return calculate_center<QPointF>(box); });
      return QPolygonF(points);
    }

    auto get_tracking_paths_to_draw(
      [[maybe_unused]] const Ui::main_window& main_window,
      const QComboBox& sequence_combobox,
      [[maybe_unused]] const QList<QAction*>& tracker_actions)
    {
      std::vector<QPolygonF> paths;
      std::vector<color_map::size_type> color_indices(1, 0);
      color_map::size_type current_index {0};
      for (const auto* action : tracker_actions)
      {
        ++current_index;
        if (action->isChecked())
        {
          paths.emplace_back(get_tracking_path_to_draw(
            application::tracking_results()[action->text().toStdString()]
                                           [sequence_combobox.currentText()
                                              .toStdString()]
                                             .bounding_boxes()));
          color_indices.push_back(current_index);
        }
      }
      return std::make_pair(paths, color_indices);
    }

    auto get_bounding_boxes_to_draw(const Ui::main_window& main_window,
                                    const QComboBox& sequence_combobox,
                                    const QList<QAction*>& tracker_actions)
    {
      analyzer::bounding_box_list boxes;
      std::vector<color_map::size_type> color_indices(1, 0);
      color_map::size_type current_index {0};
      for (const auto* action : tracker_actions)
      {
        ++current_index;
        if (action->isChecked())
        {
          boxes.emplace_back(application::tracking_result_bounding_box(
            action->text().toStdString(),
            sequence_combobox.currentText().toStdString(),
            static_cast<bounding_box_list::size_type>(
              main_window.frame_spinbox->value())));
          color_indices.push_back(current_index);
        }
      }
      return std::make_pair(boxes, color_indices);
    }

    void
    draw_boxes_on_image(QImage& image,
                        const analyzer::bounding_box_list& boxes,
                        const color_map& colors,
                        const std::vector<color_map::size_type>& color_indices)
    {
      QPainter painter {&image};
      painter.setBrush(Qt::NoBrush);
      QPen pen {Qt::red};
      pen.setWidth(2);
      for (std::array<QColor, 3>::size_type i {0};
           i < std::min(colors.size(), boxes.size());
           ++i)
      {
        pen.setColor(colors[color_indices[i]]);
        painter.setPen(pen);
        const auto box {boxes.at(i)};
        painter.drawRect(QRectF {box.x, box.y, box.width, box.height});
      }
    }

    void
    draw_paths_on_image(QImage& image,
                        const std::vector<QPolygonF>& paths,
                        const color_map& colors,
                        const std::vector<color_map::size_type>& color_indices,
                        const int current_frame)
    {
      QPainter painter {&image};
      painter.setBrush(Qt::NoBrush);
      QPen pen {Qt::red};
      pen.setWidth(3);
      QVector<QPointF> current_points;
      static constexpr int quarter_alpha {64};
      for (std::array<QColor, 3>::size_type i {0};
           i < std::min(colors.size(), paths.size());
           ++i)
      {
        auto color {colors[color_indices[i]]};
        pen.setColor(color);
        painter.setPen(pen);
        painter.drawPolyline(paths.at(i).data(), current_frame);
        pen.setColor(
          QColor {color.red(), color.green(), color.blue(), quarter_alpha});
        painter.setPen(pen);
        painter.drawPolyline(&paths.at(i)[current_frame],
                             paths[i].size() - current_frame);
        current_points.push_back(paths.at(i)[current_frame]);
      }
      // I found 12 for the pen width by trial and error.
      // NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers)
      pen.setWidth(12);
      pen.setCapStyle(Qt::RoundCap);
      for (std::array<QColor, 3>::size_type i {0};
           i < std::min(colors.size(), paths.size());
           ++i)
      {
        pen.setColor(colors[color_indices[i]]);
        painter.setPen(pen);
        painter.drawPoint(current_points[static_cast<int>(i)]);
      }
    }

    auto create_tag_label(main_window* parent,
                          const QString& tag,
                          QHBoxLayout& tag_layout)
    {
      auto* const label {new qtag {tag, parent}};
      label->setVisible(false);
      tag_layout.addWidget(label);
      return label;
    }

    auto create_tag_labels(main_window* parent, QHBoxLayout& tag_layout)
    {
      std::vector<qtag*> tag_labels;
      const auto tags {analyzer::dataset::all_tags()};
      std::transform(std::begin(tags),
                     std::end(tags),
                     std::back_inserter(tag_labels),
                     [parent, &tag_layout](const QString& tag) {
                       return create_tag_label(parent, tag, tag_layout);
                     });
      return tag_labels;
    }

    void reset_tags(const std::vector<qtag*>& tag_labels,
                    const tag_list& sequence_tags)
    {
      for (const auto& tag_label : tag_labels)
      {
        const auto shouldShow {
          std::any_of(std::begin(sequence_tags),
                      std::end(sequence_tags),
                      [tag_label](const std::string& tag) {
                        return tag == tag_label->text().toStdString();
                      })};
        tag_label->setVisible(shouldShow);
      }
    }

    auto create_dataset_info()
    {
      const auto ds {application::dataset()};
      return QString {"OTB-100\n" + QString::fromStdString(ds.root_path())
                      + "\n" + QString::number(ds.sequences().size())
                      + " sequences"};
    }

    auto make_color_map()
    {
      return color_map {analyzer::size(application::tracking_results())};
    }

    void restore_window(main_window& window)
    {
      const auto& settings {application::settings()};
      if (const auto geometry {
            settings.value(settings_keys::window_geometry, QByteArray {})
              .toByteArray()};
          !geometry.isEmpty())
      {
        window.restoreGeometry(geometry);
      }
      if (const auto state {
            settings.value(settings_keys::window_state, QByteArray {})
              .toByteArray()};
          !state.isEmpty())
      {
        window.restoreState(state);
      }
    }

    void save_window(main_window& window)
    {
      auto& settings {application::settings()};
      settings.setValue(settings_keys::window_geometry, window.saveGeometry());
      settings.setValue(settings_keys::window_state, window.saveState());
    }
  }  // namespace

  main_window::main_window(QWidget* parent):
    QMainWindow(parent),
    ui(new Ui::main_window),
    m_dataset_info_label {new QLabel {"No dataset", this}},
    m_sequence_combobox {new QComboBox {this}},
    m_draw_combobox {new QComboBox {this}}
  {
    ui->setupUi(this);
    setup_toolbar();
    setWindowTitle("");

    // change_sequence() is called by Qt during initialization. Reset the
    // frame slider and line edit enabled properties.
    ui->frame_spinbox->setEnabled(false);
    ui->frame_slider->setEnabled(false);

    ui->statusbar->addPermanentWidget(m_dataset_info_label);

    connect(ui->action_quit,
            &QAction::triggered,
            application::instance(),
            &application::quit);
    restore_window(*this);
  }

  main_window::~main_window() { delete ui; }

  void main_window::setup_toolbar()
  {
    m_sequence_combobox->addItem("Sequence");
    m_sequence_combobox->setEnabled(false);
    m_sequence_combobox->setSizeAdjustPolicy(QComboBox::AdjustToContents);
    m_sequence_combobox->setToolTip("Select the sequence to display.");
    connect(m_sequence_combobox,
            qOverload<int>(&QComboBox::currentIndexChanged),
            this,
            &main_window::change_sequence);
    ui->toolBar->addWidget(m_sequence_combobox);
    ui->toolBar->addSeparator();
    ui->toolBar->addAction(ui->action_tracker_selection);
    ui->action_tracker_selection->setEnabled(false);

    auto* const tracker_menu {new QMenu("")};
    ui->action_tracker_selection->setMenu(tracker_menu);
    dynamic_cast<QToolButton*>(
      ui->toolBar->widgetForAction(ui->action_tracker_selection))
      ->setPopupMode(QToolButton::InstantPopup);

    ui->toolBar->addWidget(m_draw_combobox);
    m_draw_combobox->addItems({"Bounding Boxes", "Tracking Path"});
    m_draw_combobox->setEnabled(false);
    connect(m_draw_combobox,
            qOverload<int>(&QComboBox::currentIndexChanged),
            this,
            &main_window::change_draw);
  }

  void main_window::open_tracking_results()
  {
    const auto results_path {QFileDialog::getExistingDirectory(
      this,
      "Load Tracking Results",
      application::settings()
        .value(settings_keys::last_loaded_results_directory, QDir::homePath())
        .toString())};
    if (!results_path.isEmpty())
    {
      load_tracking_results_directory(results_path);
    }
  }

  void main_window::open_dataset()
  {
    const auto dataset_path {QFileDialog::getExistingDirectory(
      this,
      "Load Dataset",
      application::settings()
        .value(settings_keys::last_loaded_dataset, QDir::homePath())
        .toString())};
    if (!dataset_path.isEmpty())
    {
      load_dataset(dataset_path);
    }
  }

  void main_window::change_sequence(const int index)
  {
    analyzer::gui::clear_display(*ui);
    if (index >= 0)
    {
      const auto size_index {gsl::narrow<uint64_t>(index)};
      reset_tags(
        m_tag_labels,
        application::dataset().sequences()[size_index].challenge_tags());
      analyzer::gui::synchronize_frame_controls(*ui, 0);
      draw_current_frame();
      const auto maximum_frame {gsl::narrow<int>(
        analyzer::size(application::dataset().sequences()[size_index]) - 1ul)};
      ui->frame_spinbox->setEnabled(true);
      ui->frame_spinbox->setSuffix(" of " + QString::number(maximum_frame));
      ui->frame_spinbox->setMaximum(maximum_frame);
      ui->frame_slider->setEnabled(true);
      ui->frame_slider->setMinimum(0);
      ui->frame_slider->setMaximum(maximum_frame);
      m_draw_combobox->setEnabled(true);
    }
  }

  void main_window::change_draw([[maybe_unused]] const int index)
  {
    draw_current_frame();
  }

  void main_window::change_frame(const int frame_index) const
  {
    analyzer::gui::synchronize_frame_controls(*ui, frame_index);
    draw_current_frame();
  }

  void main_window::toggle_tracker([[maybe_unused]] const bool checked)
  {
    const auto tracker_actions {
      ui->action_tracker_selection->menu()->actions()};
    for (int i {0}; i < tracker_actions.length(); ++i)
    {
      m_tracker_labels[gsl::narrow<std::vector<qtag*>::size_type>(i)]
        ->setVisible(tracker_actions[i]->isChecked());
    }
    draw_current_frame();
  }

  void main_window::load_tracking_results_directory(const QString& filepath)
  {
    setCursor(Qt::WaitCursor);
    const auto cursor_reverter {
      gsl::finally([this]() { setCursor(Qt::ArrowCursor); })};
    application::load_tracking_results(filepath);
    auto* const tracker_menu {ui->action_tracker_selection->menu()};
    tracker_menu->clear();
    m_box_colors = make_color_map();
    const auto trackers {
      analyzer::list_all_trackers(application::tracking_results())};
    for (std::vector<std::string>::size_type i {0}; i < trackers.size(); ++i)
    {
      const auto tracker_name {QString::fromStdString(trackers[i])};
      auto* const action {tracker_menu->addAction(tracker_name)};
      action->setCheckable(true);
      connect(action, &QAction::toggled, this, &main_window::toggle_tracker);
      auto* const tag {
        new qtag {tracker_name,
                  m_box_colors[gsl::narrow<color_map::size_type>(i)],
                  this}};
      tag->setVisible(false);
      ui->tracker_name_layout->addWidget(tag);
      m_tracker_labels.push_back(tag);
    }
    ui->action_tracker_selection->setEnabled(true);
    ui->statusbar->showMessage(
      "Loaded "
        + QString::number(analyzer::size(application::tracking_results()))
        + " trackers from " + filepath,
      status_bar_message_timeout.count());
  }

  namespace
  {
    auto make_tracker_tag(const std::string& tracker_name,
                          const QColor& color,
                          QLayout& layout)
    {
      auto* tag {
        new qtag {QString::fromStdString(tracker_name), color, nullptr}};
      tag->setVisible(false);
      layout.addWidget(tag);
      return tag;
    }
  }  // namespace

  auto main_window::make_tracker_action(const std::string& tracker_name)
  {
    auto* const action {ui->action_tracker_selection->menu()->addAction(
      QString::fromStdString(tracker_name))};
    action->setCheckable(true);
    connect(action, &QAction::toggled, this, &main_window::toggle_tracker);
  }

  void main_window::update_tracker_ui()
  {
    for (color_map::size_type i {0}; i < m_box_colors.size(); ++i)
    {
      m_tracker_labels.push_back(
        make_tracker_tag(application::tracking_results()[i].name(),
                         m_box_colors[i],
                         *ui->tracker_name_layout));
      make_tracker_action(application::tracking_results()[i].name());
    }
    ui->action_tracker_selection->setEnabled(true);
  }

  void main_window::load_dataset(const QString& dataset_path)
  {
    setCursor(Qt::WaitCursor);
    const auto cursor_reverter {
      gsl::finally([this]() { setCursor(Qt::ArrowCursor); })};
    // BUG Why am I loading the dataset twice?
    const auto new_dataset {
      analyzer::load_dataset_from_disk(dataset_path.toStdString())};
    if (!new_dataset.root_path().empty())
    {
      application::load_dataset(dataset_path);
      m_box_colors = make_color_map();
      ui->action_open_dataset->setEnabled(false);
      m_sequence_combobox->setEnabled(true);
      reinitialize_combobox(*m_sequence_combobox,
                            analyzer::sequence_names(application::dataset()));
      m_tag_labels = create_tag_labels(this, *ui->tag_layout);
      update_tracker_ui();
      m_dataset_info_label->setToolTip(create_dataset_info());
      m_dataset_info_label->setText("OTB-100");
    }
  }

  void main_window::closeEvent(QCloseEvent* event)
  {
    save_window(*this);
    QMainWindow::closeEvent(event);
  }

  void main_window::draw_current_frame() const
  {
    if (m_sequence_combobox->currentIndex() >= 0)
    {
      auto frame_image {application::frame_image(
        gsl::narrow<uint64_t>(m_sequence_combobox->currentIndex()),
        ui->frame_spinbox->value())};
      if (m_draw_combobox->currentIndex() == 0)
      {
        const auto [boxes, color_indices] = get_bounding_boxes_to_draw(
          *ui,
          *m_sequence_combobox,
          ui->action_tracker_selection->menu()->actions());
        draw_boxes_on_image(frame_image, boxes, m_box_colors, color_indices);
      }
      else
      {
        const auto [paths, color_indices] = get_tracking_paths_to_draw(
          *ui,
          *m_sequence_combobox,
          ui->action_tracker_selection->menu()->actions());
        draw_paths_on_image(frame_image,
                            paths,
                            m_box_colors,
                            color_indices,
                            ui->frame_spinbox->value());
      }
      ui->frame_display->setPixmap(QPixmap::fromImage(frame_image));
    }
  }
}  // namespace analyzer::gui
