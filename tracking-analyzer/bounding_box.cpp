#include "bounding_box.h"

namespace analyzer
{
  bounding_box::bounding_box(value_type x_,
                             value_type y_,
                             value_type width_,
                             value_type height_):
    x {x_}, y {y_}, width {width_}, height {height_}
  {
  }

  analyzer::overlap calculate_overlap(const analyzer::bounding_box&,
                                      const analyzer::bounding_box&)
  {
    return -1.0f;
  }

  analyzer::overlap_list calculate_overlaps(const analyzer::bounding_box_list&,
                                            const analyzer::bounding_box_list&)
  {
    return analyzer::overlap_list {};
  }

  analyzer::offset calculate_offset(const analyzer::bounding_box&,
                                    const analyzer::bounding_box&)
  {
    return -1.0f;
  }

  analyzer::offset_list calculate_offsets(const analyzer::bounding_box_list&,
                                          const analyzer::bounding_box_list&)
  {
    return analyzer::offset_list {};
  }
}  // namespace analyzer