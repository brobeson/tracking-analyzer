#include "main_window.h"
#include "application.h"
#include "ui_main_window.h"
#include <QComboBox>
#include <QDir>
#include <QFileDialog>
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

    void load_dataset(const QString& dataset_path, Ui::main_window& main_window)
    {
      // BUG Why am I loading the dataset twice?
      const auto new_dataset {analyzer::load_dataset(dataset_path)};
      if (!new_dataset.root_path().isEmpty())
      {
        application::load_dataset(dataset_path);
        main_window.sequence_combobox->setEnabled(true);
        reinitialize_combobox(
          *main_window.sequence_combobox,
          analyzer::sequence_names(
            application::instance()->dataset().sequences()));
        constexpr std::chrono::milliseconds status_bar_message_timeout {5000};
        main_window.statusbar->showMessage(
          "Loaded " + QString::number(application::dataset().sequences().size())
            + " sequences from " + dataset_path,
          status_bar_message_timeout.count());
      }
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
  }  // namespace

  main_window::main_window(QWidget* parent):
    QMainWindow(parent), ui(new Ui::main_window)
  {
    ui->setupUi(this);
    setWindowTitle("");
    if (application::settings().contains(settings_keys::last_loaded_dataset))
    {
      analyzer::gui::load_dataset(application::settings()
                                    .value(settings_keys::last_loaded_dataset)
                                    .toString(),
                                  *ui);
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
      analyzer::gui::load_dataset(dataset_path, *ui);
      setCursor(Qt::ArrowCursor);
    }
  }

  void main_window::change_sequence(const int index)
  {
    analyzer::gui::clear_display(*ui);
    if (index >= 0)
    {
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
}  // namespace analyzer::gui
