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
    using tag_list = std::vector<std::string>;

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

  //----------------------------------------------------------------------------
  //                                              old code - not refactored yet
  //                see https://github.com/brobeson/tracking-analyzer/issues/41
  //----------------------------------------------------------------------------
  class dataset final
  {
  public:
    dataset() = default;
    dataset(const QString& root_path,
            const QVector<sequence_record>& sequences);
    [[nodiscard]] auto root_path() const noexcept -> const QString&;
    [[nodiscard]] auto sequences() const noexcept
      -> const QVector<sequence_record>&;
    [[nodiscard]] auto operator[](gsl::index index) const
      -> const sequence_record&;
    [[nodiscard]] static auto all_tags() -> QStringList;

  private:
    QString m_root_directory;
    QVector<sequence_record> m_sequences;
  };

  auto load_dataset(const QString& path) -> dataset;

  auto load_ground_truth_boxes(const QString& path) -> tracker_results;

  auto sequence_names(const QVector<sequence_record>& sequences) -> QStringList;
}  // namespace analyzer

#endif