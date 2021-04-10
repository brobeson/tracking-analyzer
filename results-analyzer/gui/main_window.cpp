#include "main_window.h"
#include "file.h"
#include "ui_main_window.h"
// #include <QValueAxis>
// #include <algorithm>
// #include <filesystem>

namespace results_analyzer::gui
{
  namespace
  {
    void clear_combo_box(QComboBox& box)
    {
      while (box.count() > 0)
      {
        box.removeItem(0);
      }
    }

    void clear_list_widget(QListWidget& widget)
    {
      while (widget.count() > 0)
      {
        auto* const row {widget.takeItem(0)};
        delete row;
      }
    }

    auto load_tests(QDir results_path, const QStringList& trackers)
    {
      results_path.setNameFilters({"scores_*"});
      results_path.setFilter(QDir::Dirs);
      QStringList tests;
      for (const auto& tracker : trackers)
      {
        results_path.cd(tracker);
        tests.append(results_path.entryList());
        results_path.cdUp();
      }
      tests.removeDuplicates();
      tests.sort();
      return tests;
    }

    // NOLINTNEXTLINE(clang-diagnostic-unused-parameter)
    void set_tick_anchor(QtCharts::QValueAxis& axis, const qreal anchor)
    {
#if QT_VERSION >= QT_VERSION_CHECK(5, 12, 0)
      axis.setTickAnchor(anchor);
#else
      static_cast<void>(axis);
      static_cast<void>(anchor);
#endif
    }

    // NOLINTNEXTLINE(clang-diagnostic-unused-parameter)
    //     void set_tick_interval(QtCharts::QValueAxis& axis, const qreal
    //     interval)
    //     {
    // #if QT_VERSION >= QT_VERSION_CHECK(5, 12, 0)
    //       axis.setTickInterval(interval);
    // #else
    //       static_cast<void>(axis);
    //       static_cast<void>(anchor);
    // #endif
    //     }

    // NOLINTNEXTLINE(clang-diagnostic-unused-parameter)
    //     void set_tick_type(QtCharts::QValueAxis& axis,
    //                        //
    //                        NOLINTNEXTLINE(clang-diagnostic-unused-parameter)
    //                        const QtCharts::QValueAxis::TickType type)
    //     {
    // #if QT_VERSION >= QT_VERSION_CHECK(5, 12, 0)
    //       axis.setTickType(type);
    // #else
    //       static_cast<void>(axis);
    //       static_cast<void>(anchor);
    // #endif
    //     }

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
        set_tick_anchor(*axis, 0.0);
        axis->setTitleText("Overlap Ratio");
        return axis;
      }
      catch (...)
      {
        delete axis;
        return nullptr;
      }
    }

    // auto make_offset_y_axis(const qreal maximum) -> QtCharts::QValueAxis*
    // {
    //   QtCharts::QValueAxis* axis {nullptr};
    //   try
    //   {
    //     axis = new QtCharts::QValueAxis();
    //     axis->setRange(0.0, maximum);
    //     set_tick_anchor(*axis, 0.0);
    //     axis->setTitleText("Center Offset (pixels)");
    //     return axis;
    //   }
    //   catch (...)
    //   {
    //     delete axis;
    //     return nullptr;
    //   }
    // }

    // auto make_x_axis(const qreal maximum) -> QtCharts::QValueAxis*
    // {
    //   QtCharts::QValueAxis* axis {nullptr};
    //   try
    //   {
    //     axis = new QtCharts::QValueAxis();
    //     axis->setRange(0.0, maximum);
    //     set_tick_anchor(*axis, 0.0);
    //     // NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers)
    //     set_tick_interval(*axis, 10.0);
    //     set_tick_type(*axis, QtCharts::QValueAxis::TicksDynamic);
    //     axis->setLabelFormat("%i");
    //     axis->setTitleText("Frame Number");
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
      chart->setTitle("Overlap Ratio");
      auto* const axis {results_analyzer::gui::make_overlap_y_axis()};
      if (axis == nullptr)
      {
        chart->createDefaultAxes();
      }
      else
      {
        chart->addAxis(axis, Qt::AlignLeft);
      }
    }

    // void update_sequence_combobox(QComboBox& combobox,
    //                               const QStringList& sequence_names)
    // {
    //   combobox.clear();
    //   combobox.addItems(sequence_names);
    //   combobox.setCurrentIndex(-1);  // Ensure no item is selected.
    // }

    // void clear_display(const Ui::main_window& display)
    // {
    //   display.offset_graph->chart()->removeAllSeries();
    //   display.overlap_graph->chart()->removeAllSeries();
    //   display.frame_display->setPixmap(QPixmap {});
    //   display.frame_number->setText("");
    //   display.frame_slider->setValue(0);
    //   display.sequence_path->setText("");
    // }

    // void set_current_frame(const Ui::main_window& display, int number)
    // {
    //   display.frame_number->setText(QString::number(number));
    //   display.frame_slider->setValue(number);
    // }

    // void load_and_display_frame(const QString& frame_path, QLabel& display)
    // {
    //   QImageReader reader {frame_path};
    //   display.setPixmap(QPixmap::fromImage(reader.read()));
    // }
  }  // namespace

  main_window::main_window(QWidget* parent):
    QMainWindow(parent), ui(new Ui::main_window)
  {
    ui->setupUi(this);
    results_analyzer::gui::setup_offset_chart(ui->offset_graph->chart());
    results_analyzer::gui::setup_overlap_chart(ui->overlap_graph->chart());
    connect(ui->results_path,
            &QLineEdit::textEdited,
            this,
            &results_analyzer::gui::main_window::check_results_path);
    connect(ui->results_path,
            &QLineEdit::editingFinished,
            this,
            &results_analyzer::gui::main_window::load_trackers);
    check_results_path(ui->results_path->text());
    load_trackers();
    // load_tracking_results(ui->results_path->text());
  }

  main_window::~main_window() { delete ui; }

  void main_window::check_results_path(const QString& path_text) const
  {
    if (analyzer::path_exists(path_text))
    {
      ui->results_path->setStyleSheet("");
    }
    else
    {
      ui->results_path->setStyleSheet("QLineEdit#results_path{color:red}");
    }
  }

  void main_window::load_trackers()
  {
    const auto results_directory {
      analyzer::make_absolute_path(ui->results_path->text())};
    const auto trackers {results_directory.entryList(
      QDir::AllDirs | QDir::NoDotAndDotDot, QDir::Name)};
    results_analyzer::gui::clear_list_widget(*(ui->trackers));
    ui->trackers->addItems(trackers);
    results_analyzer::gui::clear_combo_box(*(ui->tests));
    const auto tests {results_analyzer::gui::load_tests(
      analyzer::make_absolute_path(ui->results_path->text()), trackers)};
    ui->tests->addItems(tests);
    constexpr auto message_timeout {5000};
    ui->statusbar->showMessage("Loaded " + QString::number(trackers.size())
                                 + " trackers with "
                                 + QString::number(tests.size()) + " tests.",
                               message_timeout);
  }
}  // namespace results_analyzer::gui
