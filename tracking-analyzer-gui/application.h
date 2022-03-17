#ifndef ANALYZER_APPLICATION_H
#define ANALYZER_APPLICATION_H

#include "tracking-analyzer/dataset.h"
#include "tracking-analyzer/tracking_results.h"
#include "tracking-analyzer/training_samples.h"
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
    constexpr auto last_loaded_training_directory {"recent/training_directory"};
    constexpr auto window_geometry {"window/geometry"};
    constexpr auto window_state {"window/state"};
  }  // namespace settings_keys

  class application final: public QApplication
  {
  public:
    application(int& argc, char** argv);
    [[nodiscard]] static auto instance() -> gsl::not_null<application*>;

    // Benchmark dataset
    [[nodiscard]] static auto dataset() -> analyzer::dataset&;
    static void load_dataset(const QString& dataset_path);
    [[nodiscard]] static auto frame_image(int sequence_index, int frame_index)
      -> QImage;
    [[nodiscard]] static auto ground_truth_bounding_box(int sequence_index,
                                                        int frame_index)
      -> analyzer::bounding_box;
    [[nodiscard]] static auto dataset_loaded() -> bool;

    // Application settings
    [[nodiscard]] static auto settings() -> QSettings&;

    // Tracking results
    [[nodiscard]] static auto tracking_results() -> analyzer::results_database&;
    static void load_tracking_results(const QString& results_path);
    [[nodiscard]] static auto
    tracking_result_bounding_box(const std::string& tracker_name,
                                 const std::string& sequence_name,
                                 gsl::index frame_index)
      -> analyzer::bounding_box;

    // Training sample meta-data
    static void load_training_metadata(const QString& metadata_path);
    [[nodiscard]] static auto training_samples()
      -> analyzer::training_sample_database;

  private:
    analyzer::dataset m_dataset;
    QSettings m_settings;
    analyzer::results_database m_tracking_results;
    analyzer::training_sample_database m_training_samples;
  };
}  // namespace analyzer::gui

#endif
