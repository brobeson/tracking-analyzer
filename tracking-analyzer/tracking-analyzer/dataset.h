#ifndef TRACKING_ANALYZER_DATASET_H
#define TRACKING_ANALYZER_DATASET_H

#include "tracking-analyzer/bounding_box.h"
#include <QStringList>
#include <QVector>
#include <gsl/gsl_util>
#include <stdexcept>

namespace analyzer
{
  class invalid_sequence final: public std::runtime_error
  {
  public:
    invalid_sequence(const QString& name,
                     const QString& path,
                     const std::string& what);
    [[nodiscard]] auto name() const -> QString;
    [[nodiscard]] auto path() const -> QString;

  private:
    QString m_name;
    QString m_path;
  };

  struct frame final
  {
    std::string image_path;
    analyzer::bounding_box ground_truth_bounding_box;
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
    [[nodiscard]] auto target_boxes() const -> analyzer::bounding_box_list;
    [[nodiscard]] auto tags() const -> QStringList;
    [[nodiscard]] auto operator[](gsl::index index) const -> analyzer::frame;

  private:
    QString m_name;
    QString m_root_path;
    QStringList m_frame_paths;
    analyzer::bounding_box_list m_target_boxes;
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

  auto sequence_names(const QVector<analyzer::sequence>& sequences)
    -> QStringList;
}  // namespace analyzer

#endif