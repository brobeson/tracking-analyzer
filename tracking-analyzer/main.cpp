#include "bounding_box.h"
#include "gui/main_window.h"
#include <QApplication>
#include <QScatterSeries>
#include <fstream>

int main(int argc, char* argv[])
{
  QApplication application {argc, argv};
  application.setApplicationDisplayName("Tracking Analyzer");
  application.setApplicationName("tracking-analyzer");
  application.setApplicationVersion("0.0");
  application.setOrganizationName("brobeson");

  analyzer::gui::main_window window;
  window.show();

  std::ifstream file {"~/repositories/tmft/results/otb/OTB2015/TMFT/Deer.txt"};
  const auto actual_boxes {analyzer::read_bounding_boxes(file)};
  file.close();
  file.open("~/Videos/otb/Deer/groundtruth_rect.txt");
  const auto ground_truth_boxes {analyzer::read_bounding_boxes(file)};
  file.close();

  const auto offsets {
    analyzer::calculate_offsets(actual_boxes, ground_truth_boxes)};
  auto scatter_data {new QtCharts::QScatterSeries};
  for (auto i {0u}; i < offsets.size(); ++i)
  {
    scatter_data->append(i, offsets[i]);
  }
  window.set_offset_data(scatter_data);

  const auto overlaps {
    analyzer::calculate_overlaps(actual_boxes, ground_truth_boxes)};
  auto overlap_data {new QtCharts::QScatterSeries};
  for (auto i {0u}; i < overlaps.size(); ++i)
  {
    overlap_data->append(i, overlaps[i]);
  }
  window.set_overlap_data(overlap_data);

  return application.exec();
}