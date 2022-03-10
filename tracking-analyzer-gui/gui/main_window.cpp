#include "main_window.h"
#include "application.h"
#include "ui_main_window.h"
#include <QComboBox>
#include <QDir>
#include <QFileDialog>
#include <QLabel>
#include <QPainter>

namespace analyzer::gui
{
  namespace
  {
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
      display.frame_spinbox->setValue(0);
      display.frame_slider->setValue(0);
    }

    void synchronize_frame_controls(const Ui::main_window& display, int number)
    {
      display.frame_spinbox->setValue(number);
      display.frame_slider->setValue(number);
    }

    auto get_bounding_boxes_to_draw(const Ui::main_window& main_window)
    {
      analyzer::bounding_box_list boxes;
      boxes.emplace_back(application::ground_truth_bounding_box(
        main_window.sequence_combobox->currentIndex(),
        main_window.frame_spinbox->value()));
      return boxes;
    }

    void draw_boxes_on_image(QImage& image,
                             const analyzer::bounding_box_list& boxes,
                             const std::array<QColor, 3>& colors)
    {
      QPainter painter {&image};
      painter.setBrush(Qt::NoBrush);
      QPen pen {Qt::red};
      pen.setWidth(1);
      for (std::array<QColor, 3>::size_type i {0};
           i < std::min(colors.size(), boxes.size());
           ++i)
      {
        pen.setColor(colors.at(i));
        painter.setPen(pen);
        const auto box {boxes.at(i)};
        painter.drawRect(QRectF {box.x, box.y, box.width, box.height});
      }
    }

    void draw_current_frame(const Ui::main_window& main_window,
                            const std::array<QColor, 3>& colors)
    {
      if (main_window.sequence_combobox->currentIndex() >= 0)
      {
        auto frame_image {application::frame_image(
          main_window.sequence_combobox->currentIndex(),
          main_window.frame_spinbox->value())};
        const auto boxes {get_bounding_boxes_to_draw(main_window)};
        draw_boxes_on_image(frame_image, boxes, colors);
        main_window.frame_display->setPixmap(QPixmap::fromImage(frame_image));
      }
    }

    auto create_tag_label_stylesheet(const int font_lightness)
    {
      constexpr int mid_lightness {128};
      return QString {"QLabel{border-radius: 10px; background-color: "}
             + (font_lightness < mid_lightness
                  ? QString {"rgb(200, 200, 200)}"}
                  : QString {"rgb(100, 100, 100)}"});
    }

    auto create_tag_label(main_window* parent,
                          const QString& tag,
                          QHBoxLayout& tag_layout)
    {
      auto* const label {new QLabel {tag, parent}};
      label->setFrameShape(QFrame::Panel);
      label->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
      label->setVisible(false);
      label->setMargin(4);
      label->setStyleSheet(create_tag_label_stylesheet(
        label->palette().color(QPalette::WindowText).lightness()));
      tag_layout.addWidget(label);
      return label;
    }

    auto create_tag_labels(main_window* parent, QHBoxLayout& tag_layout)
    {
      std::vector<QLabel*> tag_labels;
      const auto tags {analyzer::dataset::all_tags()};
      std::transform(std::begin(tags),
                     std::end(tags),
                     std::back_inserter(tag_labels),
                     [parent, &tag_layout](const QString& tag) {
                       return create_tag_label(parent, tag, tag_layout);
                     });
      return tag_labels;
    }

    void reset_tags(const std::vector<QLabel*>& tag_labels,
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
  }  // namespace

  main_window::main_window(QWidget* parent):
    QMainWindow(parent), ui(new Ui::main_window)
  {
    ui->setupUi(this);
    setWindowTitle("");
    if (application::settings().contains(settings_keys::last_loaded_dataset))
    {
      load_dataset(application::settings()
                     .value(settings_keys::last_loaded_dataset)
                     .toString());
    }

    // change_sequence() is called by Qt during initialization. Reset the
    // frame slider and line edit enabled properties.
    ui->frame_spinbox->setEnabled(false);
    ui->frame_slider->setEnabled(false);
  }

  main_window::~main_window() { delete ui; }

  void main_window::load_dataset()
  {
    const auto dataset_path {QFileDialog::getExistingDirectory(
      this,
      "Load Dataset",
      application::settings()
        .value(settings_keys::last_loaded_dataset, QDir::homePath())
        .toString())};
    if (!dataset_path.isEmpty())
    {
      setCursor(Qt::WaitCursor);
      load_dataset(dataset_path);
      setCursor(Qt::ArrowCursor);
    }
  }

  void main_window::change_sequence(const int index)
  {
    analyzer::gui::clear_display(*ui);
    if (index >= 0)
    {
      reset_tags(m_tag_labels, application::dataset()[index].tags());
      analyzer::gui::synchronize_frame_controls(*ui, 0);
      draw_current_frame(*ui, m_box_colors);
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
    draw_current_frame(*ui, m_box_colors);
  }

  void main_window::load_dataset(const QString& dataset_path)
  {
    // BUG Why am I loading the dataset twice?
    const auto new_dataset {analyzer::load_dataset(dataset_path)};
    if (!new_dataset.root_path().isEmpty())
    {
      application::load_dataset(dataset_path);
      ui->sequence_combobox->setEnabled(true);
      reinitialize_combobox(*ui->sequence_combobox,
                            analyzer::sequence_names(
                              application::instance()->dataset().sequences()));
      constexpr std::chrono::milliseconds status_bar_message_timeout {5000};
      ui->statusbar->showMessage(
        "Loaded " + QString::number(application::dataset().sequences().size())
          + " sequences from " + dataset_path,
        status_bar_message_timeout.count());
      m_tag_labels = create_tag_labels(this, *ui->tag_layout);
    }
  }
}  // namespace analyzer::gui
