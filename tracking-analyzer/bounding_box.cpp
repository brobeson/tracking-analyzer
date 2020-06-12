#include "bounding_box.h"
#include <QString>
#include <QStringList>
#include <cmath>

namespace analyzer
{
  namespace
  {
    auto make_bounding_box(const std::string& line)
    {
      const auto strings {QString::fromStdString(line).split(',')};
      if (strings.size() != 4)
      {
        throw std::runtime_error {
          "Found a bounding box line without 4 numbers."};
      }
      return analyzer::bounding_box {strings[0].toFloat(),
                                     strings[1].toFloat(),
                                     strings[2].toFloat(),
                                     strings[3].toFloat()};
    }

    [[nodiscard]] analyzer::bounding_box
    center(const analyzer::bounding_box box)
    {
      return analyzer::bounding_box {box.x + 0.5f * box.width,
                                     box.y + 0.5f * box.height,
                                     box.width,
                                     box.height};
    }

    [[nodiscard]] analyzer::bounding_box
    intersection(const analyzer::bounding_box a, const analyzer::bounding_box b)
    {
      const auto x {std::max(a.x, b.x)};
      const auto y {std::max(a.y, b.y)};
      return analyzer::bounding_box {x,
                                     y,
                                     std::min(a.x + a.width, b.x + b.width) - x,
                                     std::min(a.y + a.height, b.y + b.height)
                                       - y};
    }

    [[nodiscard]] float area(const analyzer::bounding_box a)
    {
      return a.width * a.height;
    }

    [[nodiscard]] bool does_intersect(const analyzer::bounding_box& a,
                                      const analyzer::bounding_box& b)
    {
      return a.x + a.width > b.x       // Box a is to the left of box b.
             && b.x + b.width > a.x    // Box b is to the left of box a.
             && a.y + a.height > b.y   // Box a is above box b.
             && b.y + b.height > a.y;  // Box b is above box a.
    }
  }  // namespace

  bounding_box::bounding_box(value_type x_,
                             value_type y_,
                             value_type width_,
                             value_type height_):
    x {x_}, y {y_}, width {width_}, height {height_}
  {
  }

  analyzer::bounding_box_list read_bounding_boxes(std::istream& stream)
  {
    std::string line;
    analyzer::bounding_box_list boxes;
    while (std::getline(stream, line))
    {
      boxes.emplace_back(analyzer::make_bounding_box(line));
    }
    return boxes;
  }

  analyzer::overlap calculate_overlap(const analyzer::bounding_box& a,
                                      const analyzer::bounding_box& b)
  {
    if (analyzer::does_intersect(a, b))
    {
      const auto i {analyzer::intersection(a, b)};
      return analyzer::area(i)
             / (analyzer::area(a) + analyzer::area(b) - analyzer::area(i));
    }
    return 0.0f;
  }

  analyzer::overlap_list
  calculate_overlaps(const analyzer::bounding_box_list& a,
                     const analyzer::bounding_box_list& b)
  {
    if (a.size() != b.size())
    {
      throw std::invalid_argument {"Bounding box lists have different sizes."};
    }
    analyzer::overlap_list overlaps;
    overlaps.reserve(a.size());
    auto ai {std::begin(a)};
    auto bi {std::begin(b)};
    while (ai != std::end(a) && bi != std::end(b))
    {
      overlaps.push_back(analyzer::calculate_overlap(*ai, *bi));
      ++ai;
      ++bi;
    }
    return overlaps;
  }

  analyzer::offset calculate_offset(const analyzer::bounding_box& a,
                                    const analyzer::bounding_box& b)
  {
    const auto centered_a {analyzer::center(a)};
    const auto centered_b {analyzer::center(b)};
    return std::sqrt(std::pow(centered_a.x - centered_b.x, 2.0f)
                     + std::pow(centered_a.y - centered_b.y, 2.0f));
  }

  analyzer::offset_list calculate_offsets(const analyzer::bounding_box_list& a,
                                          const analyzer::bounding_box_list& b)
  {
    if (a.size() != b.size())
    {
      throw std::invalid_argument {"Bounding box lists have different sizes: "
                                   + std::to_string(a.size()) + " and "
                                   + std::to_string(b.size())};
    }
    analyzer::offset_list offsets;
    offsets.reserve(a.size());
    auto ai {std::begin(a)};
    auto bi {std::begin(b)};
    while (ai != std::end(a) && bi != std::end(b))
    {
      offsets.push_back(analyzer::calculate_offset(*ai, *bi));
      ++ai;
      ++bi;
    }
    return offsets;
  }
}  // namespace analyzer