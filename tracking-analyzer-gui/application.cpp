#include "application.h"

namespace analyzer::gui
{
  application::application(int& argc, char** argv):
    QApplication {argc, argv}, m_settings {"brobeson", "tracking-analyzer"}
  {
    // Initializing m_settings also sets the application name and the,
    // organization. No need to set them here.
    QGuiApplication::setApplicationDisplayName("Tracking Analyzer");
    QCoreApplication::setApplicationVersion("0.0");
  }

  auto application::instance() -> gsl::not_null<application*>
  {
    return dynamic_cast<application*>(QCoreApplication::instance());
  }

  auto application::dataset() -> analyzer::dataset&
  {
    return instance()->m_dataset;
  }

  auto application::settings() -> QSettings& { return instance()->m_settings; }

  void application::load_dataset(const QString& dataset_path)
  {
    const auto app {application::instance()};
    app->dataset() = analyzer::load_dataset(dataset_path);
    app->settings().setValue(settings_keys::last_loaded_dataset, dataset_path);
  }

  auto application::frame_image(const int sequence_index, const int frame_index)
    -> QImage
  {
    const auto frame_path {
      application::dataset()[sequence_index][frame_index].image_path};
    QImage frame {QString::fromStdString(frame_path)};
    if (frame.format() != QImage::Format_RGB32)
    {
      frame = frame.convertToFormat(QImage::Format_RGB32);
    }
    return frame;
  }

  auto application::ground_truth_bounding_box(int sequence_index,
                                              int frame_index)
    -> analyzer::bounding_box
  {
    return application::dataset()[sequence_index][frame_index]
      .ground_truth_bounding_box;
  }

  auto application::dataset_loaded() -> bool
  {
    return !dataset().sequences().isEmpty();
  }

  auto application::tracking_results() -> analyzer::results_database&
  {
    return instance()->m_tracking_results;
  }

  void application::load_tracking_results(const QString& results_path)
  {
    const auto app {application::instance()};
    app->tracking_results()
      = analyzer::load_tracking_results_directory(results_path);
    app->settings().setValue(settings_keys::last_loaded_results_directory,
                             results_path);
  }

  auto
  application::tracking_result_bounding_box(const std::string& tracker_name,
                                            const std::string& sequence_name,
                                            const gsl::index frame_index)
    -> analyzer::bounding_box
  {
    return analyzer::get_sequence_results(
             analyzer::get_tracker_results(application::tracking_results(),
                                           tracker_name),
             sequence_name)
      .bounding_boxes[gsl::narrow_cast<analyzer::bounding_box_list::size_type>(
        frame_index)];
  }

  void application::load_training_metadata(const QString& metadata_path)
  {
    const auto app {application::instance()};
    app->m_training_samples = analyzer::load_training_samples(metadata_path);
    app->settings().setValue(settings_keys::last_loaded_training_directory,
                             metadata_path);
  }

  auto application::training_samples() -> analyzer::training_sample_database
  {
    return instance()->m_training_samples;
  }
}  // namespace analyzer::gui
