#ifndef ANALYZER_BOUNDING_BOX_H
#define ANALYZER_BOUNDING_BOX_H

#include <istream>
#include <vector>

namespace analyzer
{
  struct bounding_box final
  {
    using value_type = float;

    bounding_box() = default;
    bounding_box(value_type x,
                 value_type y,
                 value_type width,
                 value_type height);

    value_type x {0.0f};
    value_type y {0.0f};
    value_type width {0.0f};
    value_type height {0.0f};
  };

  using invalid_data = std::runtime_error;

  using bounding_box_list = std::vector<analyzer::bounding_box>;
  using overlap = float;
  using overlap_list = std::vector<analyzer::overlap>;
  using offset = float;
  using offset_list = std::vector<analyzer::offset>;

  [[nodiscard]] analyzer::bounding_box_list
  read_bounding_boxes(std::istream& stream);

  [[nodiscard]] analyzer::overlap
  calculate_overlap(const analyzer::bounding_box& a,
                    const analyzer::bounding_box& b);

  [[nodiscard]] analyzer::overlap_list
  calculate_overlaps(const analyzer::bounding_box_list& a,
                     const analyzer::bounding_box_list& b);

  [[nodiscard]] analyzer::offset
  calculate_offset(const analyzer::bounding_box& a,
                   const analyzer::bounding_box& b);

  [[nodiscard]] analyzer::offset_list
  calculate_offsets(const analyzer::bounding_box_list& a,
                    const analyzer::bounding_box_list& b);
}  // namespace analyzer

#endif
