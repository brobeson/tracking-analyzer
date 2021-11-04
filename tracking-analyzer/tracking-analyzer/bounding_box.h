#ifndef ANALYZER_BOUNDING_BOX_H
#define ANALYZER_BOUNDING_BOX_H

#include <istream>
#include <vector>

namespace analyzer
{
  struct bounding_box final
  {
    using value_type = float;
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

  [[nodiscard]] auto read_bounding_boxes(std::istream& stream)
    -> analyzer::bounding_box_list;

  // [[nodiscard]] auto calculate_overlap(const analyzer::bounding_box& a,
  //                                      const analyzer::bounding_box& b)
  //   -> analyzer::overlap;

  // [[nodiscard]] auto calculate_overlaps(const analyzer::bounding_box_list& a,
  //                                       const analyzer::bounding_box_list& b)
  //   -> analyzer::overlap_list;

  // [[nodiscard]] auto calculate_offset(const analyzer::bounding_box& a,
  //                                     const analyzer::bounding_box& b)
  //   -> analyzer::offset;

  // [[nodiscard]] auto calculate_offsets(const analyzer::bounding_box_list& a,
  //                                      const analyzer::bounding_box_list& b)
  //   -> analyzer::offset_list;
}  // namespace analyzer

#endif
