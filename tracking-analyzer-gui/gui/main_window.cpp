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
                               const QStringList& new_entries)
    {
      combobox.clear();
      combobox.addItems(new_entries);
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

    auto get_bounding_boxes_to_draw(const Ui::main_window& main_window,
                                    const QComboBox& sequence_combobox,
                                    const QList<QAction*>& tracker_actions)
    {
      analyzer::bounding_box_list boxes;
      boxes.emplace_back(application::ground_truth_bounding_box(
        sequence_combobox.currentIndex(), main_window.frame_spinbox->value()));
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
            main_window.frame_spinbox->value()));
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

    void draw_current_frame(const Ui::main_window& main_window,
                            const color_map& colors,
                            const QComboBox& sequence_combobox,
                            const QList<QAction*>& tracker_actions)
    {
      if (sequence_combobox.currentIndex() >= 0)
      {
        auto frame_image {
          application::frame_image(sequence_combobox.currentIndex(),
                                   main_window.frame_spinbox->value())};
        const auto [boxes, color_indices] = get_bounding_boxes_to_draw(
          main_window, sequence_combobox, tracker_actions);
        draw_boxes_on_image(frame_image, boxes, colors, color_indices);
        main_window.frame_display->setPixmap(QPixmap::fromImage(frame_image));
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
                    const QStringList& sequence_tags)
    {
      for (const auto& tag_label : tag_labels)
      {
        const auto shouldShow {std::any_of(std::begin(sequence_tags),
                                           std::end(sequence_tags),
                                           [tag_label](const QString& tag) {
                                             return tag == tag_label->text();
                                           })};
        tag_label->setVisible(shouldShow);
      }
    }

    auto create_dataset_info()
    {
      const auto ds {application::dataset()};
      return QString {"OTB-100\n" + ds.root_path() + "\n"
                      + QString::number(ds.sequences().length())
                      + " sequences"};
    }

    auto make_color_map()
    {
      if (application::dataset_loaded())
      {
        return color_map {1
                          + application::tracking_results().m_trackers.size()};
      }
      return color_map {0};
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
    m_sequence_combobox {new QComboBox {this}}
  {
    ui->setupUi(this);
    setup_toolbar();
    setWindowTitle("");

    // change_sequence() is called by Qt during initialization. Reset the
    // frame slider and line edit enabled properties.
    ui->frame_spinbox->setEnabled(false);
    ui->frame_slider->setEnabled(false);

    ui->statusbar->addPermanentWidget(m_dataset_info_label);

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

    connect(ui->action_quit,
            &QAction::triggered,
            application::instance(),
            &application::quit);
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
      reset_tags(m_tag_labels, application::dataset()[index].tags());
      analyzer::gui::synchronize_frame_controls(*ui, 0);
      draw_current_frame(*ui,
                         m_box_colors,
                         *m_sequence_combobox,
                         ui->action_tracker_selection->menu()->actions());
      const auto maximum_frame {
        application::dataset().sequences()[index].frame_paths().length() - 1};
      ui->frame_spinbox->setEnabled(true);
      ui->frame_spinbox->setSuffix(" of " + QString::number(maximum_frame));
      ui->frame_spinbox->setMaximum(maximum_frame);
      ui->frame_slider->setEnabled(true);
      ui->frame_slider->setMinimum(0);
      ui->frame_slider->setMaximum(maximum_frame);
    }
  }

  void main_window::change_frame(const int frame_index) const
  {
    analyzer::gui::synchronize_frame_controls(*ui, frame_index);
    draw_current_frame(*ui,
                       m_box_colors,
                       *m_sequence_combobox,
                       ui->action_tracker_selection->menu()->actions());
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
    draw_current_frame(*ui,
                       m_box_colors,
                       *m_sequence_combobox,
                       ui->action_tracker_selection->menu()->actions());
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
      analyzer::get_trackers_in_database(application::tracking_results())};
    for (QStringList::size_type i {0}; i < trackers.length(); ++i)
    {
      auto* const action {tracker_menu->addAction(trackers[i])};
      action->setCheckable(true);
      connect(action, &QAction::toggled, this, &main_window::toggle_tracker);
      auto* const tag {
        new qtag {trackers[i],
                  m_box_colors[gsl::narrow<color_map::size_type>(i + 1)],
                  this}};
      tag->setVisible(false);
      ui->tracker_name_layout->addWidget(tag);
      m_tracker_labels.push_back(tag);
    }
    ui->action_tracker_selection->setEnabled(true);
    ui->statusbar->showMessage(
      "Loaded "
        + QString::number(application::tracking_results().m_trackers.size())
        + " trackers from " + filepath,
      status_bar_message_timeout.count());
  }

  void main_window::load_dataset(const QString& dataset_path)
  {
    setCursor(Qt::WaitCursor);
    const auto cursor_reverter {
      gsl::finally([this]() { setCursor(Qt::ArrowCursor); })};
    // BUG Why am I loading the dataset twice?
    const auto new_dataset {analyzer::load_dataset(dataset_path)};
    if (!new_dataset.root_path().isEmpty())
    {
      application::load_dataset(dataset_path);
      m_box_colors = make_color_map();
      ui->action_open_dataset->setEnabled(false);
      m_sequence_combobox->setEnabled(true);
      reinitialize_combobox(*m_sequence_combobox,
                            analyzer::sequence_names(
                              application::instance()->dataset().sequences()));
      m_tag_labels = create_tag_labels(this, *ui->tag_layout);
      auto* const gt_tag {new qtag {"Ground Truth", m_box_colors[0], this}};
      ui->tracker_name_layout->addWidget(gt_tag);
      m_dataset_info_label->setToolTip(create_dataset_info());
      m_dataset_info_label->setText("OTB-100");
    }
  }

  void main_window::closeEvent(QCloseEvent* event)
  {
    save_window(*this);
    QMainWindow::closeEvent(event);
  }
}  // namespace analyzer::gui
