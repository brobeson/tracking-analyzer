#ifndef ANALYZER_APPLICATION_H
#define ANALYZER_APPLICATION_H

#include "tracking-analyzer/dataset.h"
#include <QApplication>
#include <QImage>
#include <QSettings>
#include <gsl/pointers>

namespace analyzer::gui
{
  namespace settings_keys
  {
    constexpr auto last_loaded_dataset {"recent/dataset_path"};
    constexpr auto last_loaded_results_directory {"recent/results_directory"};
  }  // namespace settings_keys

  class application final: public QApplication
  {
  public:
    application(int& argc, char** argv);
    [[nodiscard]] static auto instance() -> gsl::not_null<application*>;

    [[nodiscard]] static auto dataset() -> analyzer::dataset&;
    static void load_dataset(const QString& dataset_path);
    [[nodiscard]] static auto frame_image(int sequence_index, int frame_index)
      -> QImage;
    [[nodiscard]] static auto ground_truth_bounding_box(int sequence_index,
                                                        int frame_index)
      -> analyzer::bounding_box;

    [[nodiscard]] static auto settings() -> QSettings&;

  private:
    analyzer::dataset m_dataset;
    QSettings m_settings;
  };
}  // namespace analyzer::gui

#endif
