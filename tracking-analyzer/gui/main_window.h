#ifndef ANALYZER_GUI_MAIN_WINDOW_H
#define ANALYZER_GUI_MAIN_WINDOW_H

#include "dataset.h"
#include <QImage>
#include <QMainWindow>
#include <QScatterSeries>

namespace analyzer::gui
{
  namespace Ui
  {
    class main_window;
  }  // namespace Ui

  class main_window final: public QMainWindow
  {
    // NOLINTNEXTLINE(modernize-use-trailing-return-type)
    Q_OBJECT

  public:
    explicit main_window(QWidget* parent = nullptr);
    main_window(const main_window&) = delete;
    main_window(main_window&&) = delete;
    auto operator=(const main_window&) = delete;
    auto operator=(main_window&&) = delete;
    virtual ~main_window();  // NOLINT(modernize-use-override)

    void set_offset_data(QtCharts::QScatterSeries* data);
    void set_overlap_data(QtCharts::QScatterSeries* data);

  public slots:  // NOLINT(readability-redundant-access-specifiers)
    void check_dataset_path(const QString& path_text) const;
    void load_dataset();
    void sequence_changed(int index);
    void load_tracking_results(const QString& path);
    void change_frame(int frame_index) const;

  private:
    Ui::main_window* ui;
    analyzer::dataset m_dataset;
    int m_sequence_index {0};
  };
}  // namespace analyzer::gui

#endif  // ANALYZER_GUI_MAIN_WINDOW_H
