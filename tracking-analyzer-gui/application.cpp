#include "application.h"
#include "tracking-analyzer/filesystem.h"
#include <optional>

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

  auto application::dataset() -> analyzer::dataset_db&
  {
    return instance()->m_dataset;
  }

  auto application::settings() -> QSettings& { return instance()->m_settings; }

  void application::load_dataset(const QString& dataset_path)
  {
    const auto app {application::instance()};
    app->dataset()
      = analyzer::load_dataset_from_disk(dataset_path.toStdString());
    analyzer::push_front(app->tracking_results(),
                         analyzer::load_ground_truth_boxes(dataset_path));
    app->settings().setValue(settings_keys::last_loaded_dataset, dataset_path);
  }

  auto application::frame_image(
    const analyzer::dataset_db::sequence_list::size_type sequence_index,
    const int frame_index) -> QImage
  {
    const auto frame_path {
      application::dataset()
        .sequences()[sequence_index]
                    [static_cast<sequence_record::size_type>(frame_index)]
        .image_path()};
    QImage frame {QString::fromStdString(frame_path)};
    if (frame.format() != QImage::Format_RGB32)
    {
      frame = frame.convertToFormat(QImage::Format_RGB32);
    }
    return frame;
  }

  auto application::dataset_loaded() -> bool
  {
    return !dataset().sequences().empty();
  }

  auto application::tracking_results() -> analyzer::results_database&
  {
    return instance()->m_tracking_results;
  }

  void application::load_tracking_results(const QString& results_path)
  {
    const auto app {application::instance()};
    auto new_results {analyzer::load_tracking_results_directory(
      analyzer::make_absolute_path(results_path).toStdString())};
    if (analyzer::contains(app->tracking_results(), "Ground Truth"))
    {
      analyzer::push_front(new_results,
                           app->tracking_results()["Ground Truth"]);
    }
    app->tracking_results() = new_results;
    app->settings().setValue(settings_keys::last_loaded_results_directory,
                             results_path);
  }

  auto application::tracking_result_bounding_box(
    const std::string& tracker_name,
    const std::string& sequence_name,
    const bounding_box_list::size_type frame_index) -> analyzer::bounding_box
  {
    return application::tracking_results()[tracker_name][sequence_name]
                                          [frame_index];
  }
}  // namespace analyzer::gui
