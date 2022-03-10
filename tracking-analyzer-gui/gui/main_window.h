#ifndef ANALYZER_GUI_MAIN_WINDOW_H
#define ANALYZER_GUI_MAIN_WINDOW_H

#include <QMainWindow>
#include <array>
#include <vector>

class QLabel;

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

  public slots:  // NOLINT(readability-redundant-access-specifiers)
    void load_dataset();
    void change_sequence(int index);
    void change_frame(int frame_index) const;

  private:
    Ui::main_window* ui;
    std::array<QColor, 3> m_box_colors {Qt::red, Qt::green, Qt::blue};
    std::vector<QLabel*> m_tag_labels;
    void load_dataset(const QString& dataset_path);
  };
}  // namespace analyzer::gui

#endif  // ANALYZER_GUI_MAIN_WINDOW_H
