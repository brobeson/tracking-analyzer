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

  dataset_db::dataset_db(const std::string& root_path,
                         const sequence_list& sequences):
    m_root_path {root_path}, m_sequences {sequences}
  {
  }

  auto dataset_db::root_path() const -> std::string { return m_root_path; }

  auto dataset_db::sequences() const noexcept -> const sequence_list&
  {
    return m_sequences;
  }

  auto dataset_db::sequences() noexcept -> sequence_list&
  {
    return m_sequences;
  }

  auto dataset_db::operator[](const std::string& sequence_name) const
    -> const sequence_record&
  {
    const auto i {
      std::find_if(std::begin(m_sequences),
                   std::end(m_sequences),
                   [&sequence_name](const sequence_record& sequence) {
                     return sequence_name == sequence.name();
                   })};
    if (i == std::end(m_sequences))
    {
      throw invalid_sequence {
        sequence_name, "", sequence_name + " is not in the dataset."};
    }
    return *i;
  }

  auto dataset_db::operator[](const std::string& sequence_name)
    -> sequence_record&
  {
    const auto i {
      std::find_if(std::begin(m_sequences),
                   std::end(m_sequences),
                   [&sequence_name](const sequence_record& sequence) {
                     return sequence_name == sequence.name();
                   })};
    if (i == std::end(m_sequences))
    {
      throw invalid_sequence {
        sequence_name, "", sequence_name + " is not in the dataset."};
    }
    return *i;
  }

  auto dataset_db::challenge_tags() const -> tag_list
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
            "low resolution"};
  }

  auto begin(const dataset_db& db) -> dataset_db::sequence_list::const_iterator
  {
    return std::begin(db.sequences());
  }

  auto end(const dataset_db& db) -> dataset_db::sequence_list::const_iterator
  {
    return std::end(db.sequences());
  }

  namespace  // Functions to assist loading a dataset from disk
  {
    auto read_sequence_names(const std::string& dataset_path)
    {
      const QDir directory {QString::fromStdString(dataset_path)};
      return directory.entryList(QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name);
    }

    auto abbreviation_to_tag(const std::string& abbreviation)
    {
      // #lizard forgives
      // The other option is to use something like a map for a look up table.
      // This seems the easier choice.
      using namespace std::literals::string_literals;
      if (abbreviation == "IV")
      {
        return "illumination variation"s;  // NOLINT(cppcoreguidelines-avoid-magic-numbers)
      }
      if (abbreviation == "SV")
      {
        return "scale variation"s;  // NOLINT(cppcoreguidelines-avoid-magic-numbers)
      }
      if (abbreviation == "OCC")
      {
        return "occlusion"s;  // NOLINT(cppcoreguidelines-avoid-magic-numbers)
      }
      if (abbreviation == "DEF")
      {
        return "deformation"s;  // NOLINT(cppcoreguidelines-avoid-magic-numbers)
      }
      if (abbreviation == "MB")
      {
        return "motion blur"s;  // NOLINT(cppcoreguidelines-avoid-magic-numbers)
      }
      if (abbreviation == "FM")
      {
        return "fast motion"s;  // NOLINT(cppcoreguidelines-avoid-magic-numbers)
      }
      if (abbreviation == "IPR")
      {
        return "in-plane rotation"s;  // NOLINT(cppcoreguidelines-avoid-magic-numbers)
      }
      if (abbreviation == "OPR")
      {
        return "out-of-plane rotation"s;  // NOLINT(cppcoreguidelines-avoid-magic-numbers)
      }
      if (abbreviation == "OV")
      {
        return "out-of-view"s;  // NOLINT(cppcoreguidelines-avoid-magic-numbers)
      }
      if (abbreviation == "BC")
      {
        return "background clutters"s;  // NOLINT(cppcoreguidelines-avoid-magic-numbers)
      }
      if (abbreviation == "LR")
      {
        return "low resolution"s;  // NOLINT(cppcoreguidelines-avoid-magic-numbers)
      }
      return ""s;
    }

    auto read_sequence_tags(const std::string& sequence_path)
    {
      std::ifstream s {sequence_path + "/attrs.txt"};
      std::string abbreviation;
      tag_list tags;
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

    auto read_sequences(const std::string& dataset_path)
    {
      const auto names {read_sequence_names(dataset_path)};
      dataset_db::sequence_list sequences;
      sequences.reserve(static_cast<dataset_db::size_type>(names.size()));
      for (const auto& name : names)
      {
        try
        {
          const auto sequence_path {
            (QString::fromStdString(dataset_path) + '/' + name)};
          sequences.push_back(sequence_record {
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

  auto load_dataset_from_disk(const std::string& path) -> dataset_db
  {
    const auto dataset_path {analyzer::make_absolute_path(path)};
    return dataset_db {dataset_path, analyzer::read_sequences(dataset_path)};
  }

  auto sequence_names(const dataset_db& db) -> std::vector<std::string>
  {
    std::vector<std::string> names;
    std::transform(begin(db),
                   end(db),
                   std::back_insert_iterator {names},
                   [](const sequence_record& s) { return s.name(); });
    return names;
  }

  auto operator==(const sequence_record& sequence,
                  const std::string& name) noexcept -> bool
  {
    return sequence.name() == name;
  }

  auto contains(const dataset_db& db, const std::string& sequence_name) noexcept
    -> bool
  {
    const auto i {std::find(begin(db), end(db), sequence_name)};
    return i != end(db);
  }

  namespace  // Functions to assist loading ground truth bounding boxes
  {
    auto read_ground_truth_boxes(const std::string& path)
    {
      std::ifstream s {path + "/groundtruth_rect.txt"};
      return analyzer::read_bounding_boxes(s);
    }
  }  // namespace

  auto load_ground_truth_boxes(const QString& path) -> tracker_results
  {
    const auto dataset_path {analyzer::make_absolute_path(path)};
    const auto names {read_sequence_names(dataset_path.toStdString())};
    tracker_results gt_results {"Ground Truth", {}};
    std::transform(std::begin(names),
                   std::end(names),
                   std::back_inserter(gt_results.sequences()),
                   [&dataset_path](const QString& sequence_name) {
                     return sequence_results {
                       sequence_name.toStdString(),
                       read_ground_truth_boxes(
                         (dataset_path + '/' + sequence_name).toStdString())};
                   });
    return gt_results;
  }
}  // namespace analyzer