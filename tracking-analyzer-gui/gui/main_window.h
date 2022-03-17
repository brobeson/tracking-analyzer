#ifndef ANALYZER_GUI_MAIN_WINDOW_H
#define ANALYZER_GUI_MAIN_WINDOW_H

#include "color.h"
#include <QMainWindow>
#include <vector>

class QComboBox;
class QLabel;

namespace analyzer::gui
{
  class qtag;

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
    void load_dataset(const QString& dataset_path);

  public slots:  // NOLINT(readability-redundant-access-specifiers)
    void open_dataset();
    void open_tracking_results();
    void change_sequence(int index);
    void change_frame(int frame_index) const;
    void toggle_tracker(bool);

  protected:
    void closeEvent(QCloseEvent* event) override;

  private:
    Ui::main_window* ui;
    color_map m_box_colors {0};

    // These widgets go in the toolbar, so this class must procedurally create
    // them. In order to quickly manipulate them at runtime, keep non-owning
    // pointers to them.
    QLabel* m_dataset_info_label;
    QComboBox* m_sequence_combobox;
    void setup_toolbar();

    void load_tracking_results_directory(const QString& filepath);
    std::vector<qtag*> m_tag_labels;
    std::vector<qtag*> m_tracker_labels;
  };
}  // namespace analyzer::gui

#endif  // ANALYZER_GUI_MAIN_WINDOW_H
