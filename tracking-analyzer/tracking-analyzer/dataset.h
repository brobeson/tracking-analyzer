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
    frame_record(const std::string& img_path, bounding_box tgt_box);
    [[nodiscard]] auto image_path() const -> std::string;
    [[nodiscard]] auto target_box() const -> bounding_box;

  private:
    std::string m_image_path;
    bounding_box m_box;
  };

  class sequence_record final
  {
  public:
    [[nodiscard]] auto name() const -> std::string;
    [[nodiscard]] auto root_path() const -> std::string;
    [[nodiscard]] auto challenge_tags() const
      -> const std::vector<std::string>&;
    [[nodiscard]] auto frames() const noexcept
      -> const std::vector<frame_record>&;
    [[nodiscard]] auto frames() noexcept -> std::vector<frame_record>&;

  private:
    std::string m_name;
    std::string m_root_path;
    std::vector<std::string> m_challenge_tags;
    std::vector<frame_record> m_frames;
  };

  [[nodiscard]] inline auto begin(const sequence_record& s)
  {
    return std::begin(s.frames());
  }

  [[nodiscard]] inline auto end(const sequence_record& s)
  {
    return std::end(s.frames());
  }

  //----------------------------------------------------------------------------
  //                                              old code - not refactored yet
  //                see https://github.com/brobeson/tracking-analyzer/issues/41
  //----------------------------------------------------------------------------
  struct frame final
  {
    std::string image_path;
  };

  class sequence final
  {
  public:
    using size_type = analyzer::bounding_box_list::size_type;
    sequence() = default;
    sequence(const QString& name, const QString& path);
    [[nodiscard]] auto name() const -> QString;
    [[nodiscard]] auto frame_paths() const -> QStringList;
    [[nodiscard]] auto path() const -> QString;
    [[nodiscard]] auto tags() const -> QStringList;
    [[nodiscard]] auto operator[](gsl::index index) const -> analyzer::frame;

  private:
    QString m_name;
    QString m_root_path;
    QStringList m_frame_paths;
    QStringList m_tags;
  };

  class dataset final
  {
  public:
    dataset() = default;
    dataset(const QString& root_path,
            const QVector<analyzer::sequence>& sequences);
    [[nodiscard]] auto root_path() const noexcept -> const QString&;
    [[nodiscard]] auto sequences() const noexcept
      -> const QVector<analyzer::sequence>&;
    [[nodiscard]] auto operator[](gsl::index index) const
      -> const analyzer::sequence&;
    [[nodiscard]] static auto all_tags() -> QStringList;

  private:
    QString m_root_directory;
    QVector<analyzer::sequence> m_sequences;
  };

  auto load_dataset(const QString& path) -> dataset;

  auto load_ground_truth_boxes(const QString& path) -> tracker_results;

  auto sequence_names(const QVector<analyzer::sequence>& sequences)
    -> QStringList;
}  // namespace analyzer

#endif