#include "gui/main_window.h"
#include <QApplication>
#include <QImageReader>
#include <QScatterSeries>
#include <fstream>

auto main(int argc, char* argv[]) -> int
{
  QApplication application {argc, argv};
  QGuiApplication::setApplicationDisplayName("Tracking Analyzer");
  QCoreApplication::setApplicationName("tracking-analyzer");
  QCoreApplication::setApplicationVersion("0.0");
  QCoreApplication::setOrganizationName("brobeson");

  analyzer::gui::main_window window;
  window.show();
  return QCoreApplication::exec();
}

// std::ifstream file
// {"~/repositories/tmft/results/otb/OTB2015/TMFT/Deer.txt"}; const auto
// actual_boxes {analyzer::read_bounding_boxes(file)}; file.close();
// file.open("~/Videos/otb/Deer/groundtruth_rect.txt");
// const auto ground_truth_boxes {analyzer::read_bounding_boxes(file)};
// file.close();

// const auto offsets {
//   analyzer::calculate_offsets(actual_boxes, ground_truth_boxes)};
// auto* const scatter_data {new QtCharts::QScatterSeries};
// for (auto i {0u}; i < offsets.size(); ++i)
// {
//   scatter_data->append(i, offsets[i]);
// }
// window.set_offset_data(scatter_data);

// const auto overlaps {
//   analyzer::calculate_overlaps(actual_boxes, ground_truth_boxes)};
// auto* const overlap_data {new QtCharts::QScatterSeries};
// for (auto i {0u}; i < overlaps.size(); ++i)
// {
//   overlap_data->append(i, overlaps[i]);
// }
// window.set_overlap_data(overlap_data);

// QImageReader reader {"~/Videos/otb/Deer/img/0001.jpg"};
// const auto image {reader.read()};
// window.set_image(image);
