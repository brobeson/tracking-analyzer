#ifndef TRACKING_ANALYZER_DATASET_H
#define TRACKING_ANALYZER_DATASET_H

#include "tracking-analyzer/bounding_box.h"
#include "tracking-analyzer/exceptions.h"
#include "tracking-analyzer/tracking_results.h"
#include <QStringList>
#include <QVector>
#include <gsl/gsl_util>

namespace analyzer
{
  using tag_list = std::vector<std::string>;
  using name_list = tag_list;

  class frame_record final
  {
  public:
    frame_record() = default;
    explicit frame_record(const std::string& img_path);
    [[nodiscard]] auto image_path() const -> std::string;

  private:
    std::string m_image_path;
  };

  class sequence_record final
  {
  public:
    using frame_list = std::vector<frame_record>;
    using size_type = frame_list::size_type;

    sequence_record() = default;
    sequence_record(const std::string& sequence_name,
                    const std::string& sequence_path,
                    const tag_list& challenges,
                    const frame_list& frame_data);
    [[nodiscard]] auto name() const -> std::string;
    [[nodiscard]] auto root_path() const -> std::string;
    [[nodiscard]] auto challenge_tags() const -> const tag_list&;
    [[nodiscard]] auto frames() const noexcept -> const frame_list&;
    [[nodiscard]] auto frames() noexcept -> frame_list&;
    [[nodiscard]] auto operator[](size_type i) const -> const frame_record&;
    [[nodiscard]] auto operator[](size_type i) -> frame_record&;

  private:
    std::string m_name;
    std::string m_root_path;
    tag_list m_challenge_tags;
    frame_list m_frames;
  };

  [[nodiscard]] auto size(const sequence_record& seq)
    -> sequence_record::size_type;

  [[nodiscard]] auto begin(const sequence_record& s)
    -> sequence_record::frame_list::const_iterator;

  [[nodiscard]] auto end(const sequence_record& s)
    -> sequence_record::frame_list::const_iterator;

  class dataset
  {
  public:
    using sequence_list = std::vector<sequence_record>;
    using size_type = sequence_list::size_type;

    dataset() = default;
    dataset(const std::string& root_path, const sequence_list& sequences);
    dataset(const dataset&) = default;
    dataset(dataset&&) = default;
    virtual ~dataset() = default;
    auto operator=(const dataset&) -> dataset& = default;
    auto operator=(dataset &&) -> dataset& = default;
    [[nodiscard]] virtual auto name() const -> std::string;
    [[nodiscard]] auto root_path() const -> std::string;
    [[nodiscard]] auto sequences() const noexcept -> const sequence_list&;
    [[nodiscard]] auto sequences() noexcept -> sequence_list&;
    [[nodiscard]] auto operator[](const std::string& sequence_name) const
      -> const sequence_record&;
    [[nodiscard]] auto operator[](const std::string& sequence_name)
      -> sequence_record&;
    [[nodiscard]] virtual auto challenge_tags() const -> tag_list;

  private:
    std::string m_root_path;
    sequence_list m_sequences;
  };

  [[nodiscard]] auto begin(const dataset& db)
    -> dataset::sequence_list::const_iterator;
  [[nodiscard]] auto end(const dataset& db)
    -> dataset::sequence_list::const_iterator;

  [[nodiscard]] auto load_dataset_from_disk(const std::string& path) -> dataset;

  auto operator==(const sequence_record& sequence,
                  const std::string& name) noexcept -> bool;
  auto contains(const dataset& db, const std::string& sequence_name) noexcept
    -> bool;

  auto sequence_names(const dataset& db) -> name_list;

  auto load_ground_truth_boxes(const QString& path) -> tracker_results;
}  // namespace analyzer

#endif