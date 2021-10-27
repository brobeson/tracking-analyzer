#ifndef ANALYZER_GUI_MAIN_WINDOW_H
#define ANALYZER_GUI_MAIN_WINDOW_H

#include "dataset.h"
#include "tracking_results.h"
#include "training_metadata.h"
#include <QImage>
#include <QMainWindow>
#include <QScatterSeries>
#include <QSettings>

class QSpinBox;
class QComboBox;

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

    void set_overlap_data(QtCharts::QScatterSeries* bg_candidate_data,
                          QtCharts::QScatterSeries* bg_mined_data,
                          QtCharts::QScatterSeries* tg_candidate_data);

    // TODO Can this be a free function in main_window.cpp?
    void set_training_score_data(const training_batch& batch);

  public slots:  // NOLINT(readability-redundant-access-specifiers)
    void load_dataset();
    void sequence_changed(int index);
    // void load_tracking_results(const QString& path);
    void change_frame(int frame_index) const;
    void load_tracking_data(bool);
    void change_update(int update_number);
    void change_point_size(int size) const;
    void toggle_bg_candidate_plot(bool checked) const;
    void toggle_bg_mined_plot(bool checked) const;
    void toggle_tg_candidate_plot(bool checked) const;
    void save_graph(bool);
    void change_training_batch(int batch_number);

  private:
    Ui::main_window* ui;
    analyzer::dataset m_dataset;
    int m_sequence_index {0};
    analyzer::training_scores m_training_data;
    analyzer::training_iterator m_current_training;
    QSettings settings;

    QSpinBox* m_update_spinbox {nullptr};
    QSpinBox* m_batch_spinbox {nullptr};
    QComboBox* m_sequence_combobox {nullptr};
    QSpinBox* m_point_size_spinbox {nullptr};
    void add_graph_controls_to_toolbar();
    void load_dataset(const QString& dataset_path);
  };
}  // namespace analyzer::gui

#endif  // ANALYZER_GUI_MAIN_WINDOW_H
