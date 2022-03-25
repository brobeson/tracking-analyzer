#include "tracking-analyzer/dataset.h"
#include "tracking-analyzer/filesystem.h"
#include <QDir>
#include <filesystem>
#include <fstream>
#include <gsl/gsl_assert>

namespace analyzer
{
  frame_record::frame_record(const std::string& img_path):
    m_image_path {img_path}
  {
  }

  auto frame_record::image_path() const -> std::string { return m_image_path; }

  sequence_record::sequence_record(const std::string& sequence_name,
                                   const std::string& sequence_path,
                                   const tag_list& challenges,
                                   const frame_list& frame_data):
    m_name {sequence_name},
    m_root_path {sequence_path},
    m_challenge_tags {challenges},
    m_frames {frame_data}
  {
  }

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

  auto sequence_record::operator[](size_type i) const -> const frame_record&
  {
    return m_frames.at(i);
  }

  auto sequence_record::operator[](size_type i) -> frame_record&
  {
    return m_frames.at(i);
  }

  auto size(const sequence_record& seq) -> sequence_record::size_type
  {
    return seq.frames().size();
  }

  auto begin(const sequence_record& s)
    -> sequence_record::frame_list::const_iterator
  {
    return std::begin(s.frames());
  }

  auto end(const sequence_record& s)
    -> sequence_record::frame_list::const_iterator
  {
    return std::end(s.frames());
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

    auto make_sequence_frame_paths(const QString& sequence_path)
    {
      QDir directory {sequence_path};
      if (!directory.cd("img"))
      {
        return sequence_record::frame_list {};
      }
      auto frame_paths {
        directory.entryList({"*.jpg"}, QDir::Files, QDir::Name)};
      for (auto& frame_path : frame_paths)
      {
        frame_path.prepend('/').prepend(directory.absolutePath());
      }
      sequence_record::frame_list frames;
      std::transform(std::begin(frame_paths),
                     std::end(frame_paths),
                     std::back_inserter(frames),
                     [](const QString& frame_path) {
                       return frame_record {frame_path.toStdString()};
                     });
      return frames;
    }

    auto read_ground_truth_boxes(const std::string& path)
    {
      std::ifstream s {path + "/groundtruth_rect.txt"};
      return analyzer::read_bounding_boxes(s);
    }

    auto read_ground_truth_boxes(const QString& path)
    {
      return read_ground_truth_boxes(path.toStdString());
    }

    auto abbreviation_to_tag(const std::string& abbreviation)
    {
      // #lizard forgives
      // The other option is to use something like a map for a look up table.
      // This seems the easier choice.
      using namespace std::literals::string_literals;
      if (abbreviation == "IV")
      {
        return "illumination variation"s;
      }
      if (abbreviation == "SV")
      {
        return "scale variation"s;
      }
      if (abbreviation == "OCC")
      {
        return "occlusion"s;
      }
      if (abbreviation == "DEF")
      {
        return "deformation"s;
      }
      if (abbreviation == "MB")
      {
        return "motion blur"s;
      }
      if (abbreviation == "FM")
      {
        return "fast motion"s;
      }
      if (abbreviation == "IPR")
      {
        return "in-plane rotation"s;
      }
      if (abbreviation == "OPR")
      {
        return "out-of-plane rotation"s;
      }
      if (abbreviation == "OV")
      {
        return "out-of-view"s;
      }
      if (abbreviation == "BC")
      {
        return "background clutters"s;
      }
      if (abbreviation == "LR")
      {
        return "low resolution"s;
      }
      return ""s;
    }

    auto read_sequence_tags(const std::string& sequence_path)
    {
      std::ifstream s {sequence_path + "/attrs.txt"};
      std::string abbreviation;
      sequence_record::tag_list tags;
      while (s)
      {
        s >> abbreviation;
        if (abbreviation.back() == ',')
        {
          abbreviation.pop_back();
        }
        const auto tag {abbreviation_to_tag(abbreviation)};
        if (!tag.empty())
        {
          tags.push_back(tag);
        }
      }
      std::sort(std::begin(tags), std::end(tags));
      return tags;
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
          const auto sequence_path {(dataset_path + '/' + name)};
          sequences.push_back(analyzer::sequence {
            name.toStdString(),
            sequence_path.toStdString(),
            analyzer::read_sequence_tags(sequence_path.toStdString()),
            analyzer::make_sequence_frame_paths(sequence_path)});
        }
        catch (...)
        {
          // Do nothing for now. Maybe try to report it in the future.
        }
      }
      return sequences;
    }
  }  // namespace

  // sequence::sequence(const QString& name, const QString& path):
  //   m_name {name},
  //   m_root_path {path},
  //   m_frame_paths {analyzer::make_sequence_frame_paths(path)},
  //   m_tags {analyzer::read_sequence_tags(path.toStdString())}
  // {
  //   if (m_root_path.isEmpty())
  //   {
  //     throw analyzer::invalid_sequence {
  //       m_name.toStdString(),
  //       m_root_path.toStdString(),
  //       "A sequence cannot have an empty root path."};
  //   }
  //   if (!QDir {m_root_path}.exists())
  //   {
  //     throw analyzer::invalid_sequence {
  //       m_name.toStdString(),
  //       m_root_path.toStdString(),
  //       "The sequence path " + m_root_path.toStdString() + " does not
  //       exist."};
  //   }
  //   if (m_frame_paths.isEmpty())
  //   {
  //     throw analyzer::invalid_sequence {
  //       m_name.toStdString(),
  //       m_root_path.toStdString(),
  //       "The sequence " + m_name.toStdString() + " at path "
  //         + m_root_path.toStdString() + " does not have frame images."};
  //   }
  // }

  // auto sequence::name() const -> QString { return m_name; }
  // auto sequence::frame_paths() const -> QStringList { return m_frame_paths; }
  // auto sequence::path() const -> QString { return m_root_path; }
  // auto sequence::tags() const -> QStringList { return m_tags; }

  // auto sequence::operator[](gsl::index index) const -> frame_record
  // {
  //   return analyzer::frame_record {
  //     m_frame_paths[gsl::narrow_cast<int>(index)].toStdString()};
  // }

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

  auto load_ground_truth_boxes(const QString& path) -> tracker_results
  {
    const auto dataset_path {analyzer::make_absolute_path(path)};
    const auto names {read_sequence_names(dataset_path)};
    tracker_results gt_results {"Ground Truth", {}};
    std::transform(
      std::begin(names),
      std::end(names),
      std::back_inserter(gt_results.sequences()),
      [&dataset_path](const QString& sequence_name) {
        return sequence_results {
          sequence_name.toStdString(),
          read_ground_truth_boxes(dataset_path + '/' + sequence_name)};
      });
    return gt_results;
  }

  auto load_dataset(const QString& path) -> dataset
  {
    const auto dataset_path {analyzer::make_absolute_path(path)};
    return dataset {dataset_path, analyzer::read_sequences(dataset_path)};
  }

  auto sequence_names(const QVector<analyzer::sequence>& sequences)
    -> QStringList
  {
    QStringList names;
    std::transform(std::begin(sequences),
                   std::end(sequences),
                   std::back_insert_iterator {names},
                   [](const analyzer::sequence& s) {
                     return QString::fromStdString(s.name());
                   });
    return names;
  }
}  // namespace analyzer