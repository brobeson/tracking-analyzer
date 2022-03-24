#include "tracking-analyzer/dataset.h"
#include "tracking-analyzer/filesystem.h"
#include "tracking-analyzer/tracking_results.h"
#include <QDir>
#include <filesystem>
#include <fstream>
#include <gsl/gsl_assert>

namespace analyzer
{
  frame_record::frame_record(const std::string& img_path, bounding_box tgt_box):
    m_image_path {img_path}, m_box {tgt_box}
  {
  }
  auto frame_record::image_path() const -> std::string { return m_image_path; }
  auto frame_record::target_box() const -> bounding_box { return m_box; }

  auto sequence_record::name() const -> std::string { return m_name; }
  auto sequence_record::root_path() const -> std::string { return m_root_path; }
  auto sequence_record::challenge_tags() const
    -> const std::vector<std::string>&
  {
    return m_challenge_tags;
  }
  auto sequence_record::frames() const noexcept
    -> const std::vector<frame_record>&
  {
    return m_frames;
  }
  auto sequence_record::frames() noexcept -> std::vector<frame_record>&
  {
    return m_frames;
  }

  //----------------------------------------------------------------------------
  //                                              old code - not refactored yet
  //                see https://github.com/brobeson/tracking-analyzer/issues/41
  //----------------------------------------------------------------------------
  namespace
  {
    auto read_sequence_names(const QString& dataset_path)
    {
      const QDir directory {dataset_path};
      return directory.entryList(QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name);
    }

    auto read_sequences(const QString& dataset_path)
    {
      const auto names {read_sequence_names(dataset_path)};
      QVector<analyzer::sequence> sequences;
      sequences.reserve(names.size());
      for (const auto& name : names)
      {
        try
        {
          sequences.push_back(
            analyzer::sequence {name, dataset_path + '/' + name});
        }
        catch (...)
        {
          // Do nothing for now. Maybe try to report it in the future.
        }
      }
      return sequences;
    }

    auto make_sequence_frame_paths(const QString& sequence_path)
    {
      QDir directory {sequence_path};
      if (!directory.cd("img"))
      {
        return QStringList {};
      }
      auto frame_paths {
        directory.entryList({"*.jpg"}, QDir::Files, QDir::Name)};
      for (auto& frame_path : frame_paths)
      {
        frame_path.prepend('/').prepend(directory.absolutePath());
      }
      return frame_paths;
    }

    auto read_ground_truth_boxes(const std::string& path)
    {
      std::ifstream s {path + "/groundtruth_rect.txt"};
      return analyzer::read_bounding_boxes(s);
    }

    auto read_ground_truth_boxes(const std::string& dataset_path,
                                 const std::string& sequence_name)
    {
      return read_ground_truth_boxes(dataset_path + '/' + sequence_name);
    }

    auto abbreviation_to_tag(const std::string& abbreviation)
    {
      // #lizard forgives
      // The other option is to use something like a map for a look up table.
      // This seems the easier choice.
      if (abbreviation == "IV")
      {
        return "illumination variation";
      }
      if (abbreviation == "SV")
      {
        return "scale variation";
      }
      if (abbreviation == "OCC")
      {
        return "occlusion";
      }
      if (abbreviation == "DEF")
      {
        return "deformation";
      }
      if (abbreviation == "MB")
      {
        return "motion blur";
      }
      if (abbreviation == "FM")
      {
        return "fast motion";
      }
      if (abbreviation == "IPR")
      {
        return "in-plane rotation";
      }
      if (abbreviation == "OPR")
      {
        return "out-of-plane rotation";
      }
      if (abbreviation == "OV")
      {
        return "out-of-view";
      }
      if (abbreviation == "BC")
      {
        return "background clutters";
      }
      if (abbreviation == "LR")
      {
        return "low resolution";
      }
      return "";
    }

    auto read_sequence_tags(const std::string& sequence_path)
    {
      std::ifstream s {sequence_path + "/attrs.txt"};
      std::string abbreviation;
      QStringList tags;
      while (s)
      {
        s >> abbreviation;
        if (abbreviation.back() == ',')
        {
          abbreviation.pop_back();
        }
        const QString tag {abbreviation_to_tag(abbreviation)};
        if (!tag.isEmpty())
        {
          tags << tag;
        }
      }
      tags.sort();
      return tags;
    }
  }  // namespace

  sequence::sequence(const QString& name, const QString& path):
    m_name {name},
    m_root_path {path},
    m_frame_paths {analyzer::make_sequence_frame_paths(path)},
    m_target_boxes {analyzer::read_ground_truth_boxes(path.toStdString())},
    m_tags {analyzer::read_sequence_tags(path.toStdString())}
  {
    if (m_root_path.isEmpty())
    {
      throw analyzer::invalid_sequence {
        m_name.toStdString(),
        m_root_path.toStdString(),
        "A sequence cannot have an empty root path."};
    }
    if (!QDir {m_root_path}.exists())
    {
      throw analyzer::invalid_sequence {
        m_name.toStdString(),
        m_root_path.toStdString(),
        "The sequence path " + m_root_path.toStdString() + " does not exist."};
    }
    if (m_frame_paths.isEmpty())
    {
      throw analyzer::invalid_sequence {
        m_name.toStdString(),
        m_root_path.toStdString(),
        "The sequence " + m_name.toStdString() + " at path "
          + m_root_path.toStdString() + " does not have frame images."};
    }
  }

  auto sequence::name() const -> QString { return m_name; }
  auto sequence::frame_paths() const -> QStringList { return m_frame_paths; }
  auto sequence::path() const -> QString { return m_root_path; }
  auto sequence::target_boxes() const -> analyzer::bounding_box_list
  {
    return m_target_boxes;
  }
  auto sequence::tags() const -> QStringList { return m_tags; }

  auto sequence::operator[](gsl::index index) const -> analyzer::frame
  {
    Expects(index >= 0
            && index < gsl::narrow_cast<gsl::index>(m_target_boxes.size()));
    return analyzer::frame {
      m_frame_paths[gsl::narrow_cast<int>(index)].toStdString(),
      m_target_boxes[gsl::narrow_cast<sequence::size_type>(index)]};
  }

  dataset::dataset(const QString& root_path,
                   const QVector<sequence>& sequences):
    m_root_directory {root_path}, m_sequences {sequences}
  {
  }

  auto dataset::sequences() const noexcept -> const QVector<analyzer::sequence>&
  {
    return m_sequences;
  }

  auto dataset::root_path() const noexcept -> const QString&
  {
    return m_root_directory;
  }

  auto dataset::all_tags() -> QStringList
  {
    return {"illumination variation",
            "scale variation",
            "occlusion",
            "deformation",
            "motion blur",
            "fast motion",
            "in-plane rotation",
            "out-of-plane rotation",
            "out-of-view",
            "background clutters",
            "low resolution"

    };
  }

  auto dataset::operator[](const gsl::index index) const
    -> const analyzer::sequence&
  {
    Expects(index >= 0 && index < m_sequences.length());
    return m_sequences[gsl::narrow_cast<int>(index)];
  }

  namespace
  {
    auto load_ground_truth_boxes(const std::string& dataset_path)
    {
      const auto names {
        read_sequence_names(QString::fromStdString(dataset_path))};
      tracker_results gt_results {"Ground Truth", {}};
      std::transform(
        std::begin(names),
        std::end(names),
        std::back_inserter(gt_results.sequences()),
        [&dataset_path](const QString& sequence_name) {
          return sequence_results {
            sequence_name.toStdString(),
            read_ground_truth_boxes(dataset_path, sequence_name.toStdString())};
        });
      return gt_results;
    }
  }  // namespace

  auto load_dataset(const QString& path) -> analyzer::dataset
  {
    const auto dataset_path {analyzer::make_absolute_path(path)};
    return analyzer::dataset {dataset_path,
                              analyzer::read_sequences(dataset_path)};
  }

  auto sequence_names(const QVector<analyzer::sequence>& sequences)
    -> QStringList
  {
    QStringList names;
    std::transform(std::begin(sequences),
                   std::end(sequences),
                   std::back_insert_iterator {names},
                   [](const analyzer::sequence& s) { return s.name(); });
    return names;
  }
}  // namespace analyzer