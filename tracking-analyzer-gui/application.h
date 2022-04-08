#ifndef ANALYZER_APPLICATION_H
#define ANALYZER_APPLICATION_H

#include "tracking-analyzer/dataset.h"
#include "tracking-analyzer/tracking_results.h"
#include <QApplication>
#include <QImage>
#include <QSettings>
#include <gsl/pointers>

namespace analyzer::gui
{
  namespace settings_keys
  {
    static constexpr auto last_loaded_dataset {"recent/dataset_path"};
    static constexpr auto last_loaded_results_directory {
      "recent/results_directory"};
    static constexpr auto window_geometry {"window/geometry"};
    static constexpr auto window_state {"window/state"};
  }  // namespace settings_keys

  class application final: public QApplication
  {
  public:
    application(int& argc, char** argv);
    [[nodiscard]] static auto instance() -> gsl::not_null<application*>;

    [[nodiscard]] static auto dataset() -> analyzer::dataset&;
    static void load_dataset(const QString& dataset_path);
    [[nodiscard]] static auto
    frame_image(analyzer::dataset::sequence_list::size_type sequence_index,
                int frame_index) -> QImage;
    [[nodiscard]] static auto dataset_loaded() -> bool;

    [[nodiscard]] static auto settings() -> QSettings&;

    [[nodiscard]] static auto tracking_results() -> analyzer::results_database&;
    static void load_tracking_results(const QString& results_path);
    [[nodiscard]] static auto
    tracking_result_bounding_box(const std::string& tracker_name,
                                 const std::string& sequence_name,
                                 bounding_box_list::size_type frame_index)
      -> analyzer::bounding_box;

  private:
    analyzer::dataset m_dataset;
    QSettings m_settings;
    analyzer::results_database m_tracking_results;
  };
}  // namespace analyzer::gui

#endif
