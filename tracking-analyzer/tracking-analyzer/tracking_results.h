#ifndef ANALYZER_TRACKING_RESULTS_H
#define ANALYZER_TRACKING_RESULTS_H

#include "tracking-analyzer/bounding_box.h"
#include "tracking-analyzer/exceptions.h"
#include <QList>
#include <QString>
#include <array>
#include <utility>
#include <vector>

namespace analyzer
{
  /**
   * \brief Encapsulate tracking results for one sequence.
   * \details The sequence_results class collects together the results for one
   * tracker on one sequence within one benchmark dataset. A sequence_results
   * object provides the name of the sequence and the target bounding boxes
   * reported by the tracker. At this time, the class only supports axis-aligned
   * bounding boxes. It also does not support tombstones denoting target loss
   * and tracker re-initialization.
   */
  class sequence_results final
  {
  public:
    /// Construct sequence results with no name and no bounding boxes.
    sequence_results() = default;

    /**
     * \brief Construct a new sequence_results object
     * \param[in] sequence_name The name of the sequence. This must
     *    case-sensitively match a sequence name in the benchmark dataset. For
     *    example, "deer" will not work for OTB results because the correct name
     *    is "Deer".
     * \param[in] target_bounding_boxes The bounding boxes reported by the
     *    tracker for this sequence. The number of boxes should equal the number
     *    of tracked frames in the sequence.
     * \warning The sequence_results class cannot validate the sequence name or
     *    the bounding boxes. It does not have any information about loaded
     *    datasets.
     */
    sequence_results(const std::string& sequence_name,
                     const bounding_box_list& target_bounding_boxes);

    /// \brief Get the name of the sequence.
    [[nodiscard]] auto name() const -> std::string;

    /// \brief Get read-only access to the sequence's target bounding boxes.
    [[nodiscard]] auto bounding_boxes() const noexcept
      -> const bounding_box_list&;

    /// \brief Get read-write access to the sequences target bounding boxes.
    [[nodiscard]] auto bounding_boxes() noexcept -> bounding_box_list&;

    /**
     * \brief Get read-only access to a specific target bounding box.
     * \param[in] i The index into the bounding box list. This is not a frame
     *    number; it's a 0-based index like for any other C++ contiguous data
     *    structure.
     * \return A read-only reference to the requested bounding box.
     * \throws std::out_of_range If \$i \ge size()\$.
     */
    [[nodiscard]] auto operator[](bounding_box_list::size_type i) const
      -> const bounding_box&;

    /**
     * \brief Get read-write access to a specific target bounding box.
     * \param[in] i The index into the bounding box list. This is not a frame
     *    number; it's a 0-based index like for any other C++ contiguous data
     *    structure.
     * \return A read-write reference to the requested bounding box.
     * \throws std::out_of_range If \$i \ge size()\$.
     */
    [[nodiscard]] auto operator[](bounding_box_list::size_type i)
      -> bounding_box&;

  private:
    std::string m_name;
    bounding_box_list m_target_boxes;
  };

  /**
   * \brief Get the number of target bounding boxes in a sequence.
   * \param[in] sequence Get the number of bounding boxes for this sequence.
   * \return The number of bounding boxes in \a sequence.
   * \details This behaves as if you called
   * \code {.cpp}
   * sequence.bounding_boxes().size();
   * \endcode
   * \related sequence_results
   */
  [[nodiscard]] auto size(const sequence_results& sequence) noexcept
    -> bounding_box_list::size_type;

  /**
   * \brief Get an iterator to the first bounding box in a sequence.
   * \param[in] sequence Get an iterator for this sequence.
   * \return An iterator to the first bounding box in \a sequence.
   * \details This allows client code to use a sequence_results object in a
   * range-for loop or iterator-style algorithms:
   * \code {.cpp}
   * std::any_of(
   *   analyzer::begin(sequence),
   *   analyzer::end(sequence),
   *   [](const analyzer::bounding_box& box) {
   *     return box.width * box.height == 0;});
   * for (const auto& box : sequence) {
   *   // Draw the box.
   * }
   * \endcode
   * \related sequence_results
   */
  [[nodiscard]] auto begin(const sequence_results& sequence)
    -> bounding_box_list::const_iterator;

  /**
   * \brief Get a past-the-end iterator to a sequences' bounding boxes.
   * \param[in] sequence Get an iterator for this sequence.
   * \return A past-the-end iterator to the \a sequence's bounding boxes.
   * \see begin()
   * \related sequence_results
   */
  [[nodiscard]] auto end(const sequence_results& sequence)
    -> bounding_box_list::const_iterator;

  /**
   * \brief Encapsulate tracking results for one tracker.
   * \details The tracker_results class collects together the results for one
   * tracker within one benchmark dataset. A tracker_results object provides the
   * name of the tracker and the results for each sequence in the dataset.
   */
  class tracker_results final
  {
  public:
    /// A container of sequence_results objects. This is a [contiguous
    /// container](https://en.cppreference.com/w/cpp/named_req/ContiguousContainer).
    using sequence_list = std::vector<sequence_results>;
    using size_type = sequence_list::size_type;

    /// Construct tracker results with no tracker name and no sequence results.
    tracker_results() = default;

    /**
     * \brief Construct a new tracker_results object
     * \param[in] tracker_name The name of the tracker. Tracker names are case
     *    sensitive.
     * \param[in] tracked_sequences The set of sequence results reported by this
     *    tracker.
     */
    tracker_results(const std::string& tracker_name,
                    const sequence_list& tracked_sequences);

    /// Get the name of the tracker.
    [[nodiscard]] auto name() const -> std::string;

    /// Get read-only access to the set of sequence results.
    [[nodiscard]] auto sequences() const noexcept -> const sequence_list&;

    /// Get read-write access to the set of sequence results.
    [[nodiscard]] auto sequences() noexcept -> sequence_list&;

    /**
     * \brief Get read-only access to the results for a specific sequence.
     * \param[in] i The index into the sequence results list. This is a
     *    0-based index like for any other C++ contiguous data structure.
     * \return A read-only reference to the requested sequence results.
     * \throws std::out_of_range If \$i \ge size()\$.
     */
    [[nodiscard]] auto operator[](size_type i) const -> const sequence_results&;

    /**
     * \brief Get read-write access to the results for a specific sequence.
     * \param[in] i The index into the sequence results list. This is a
     *    0-based index like for any other C++ contiguous data structure.
     * \return A read-write reference to the requested sequence results.
     * \throws std::out_of_range If \$i \ge size()\$.
     */
    [[nodiscard]] auto operator[](size_type i) -> sequence_results&;

    /**
     * \brief Get read-only access to the results for a specific sequence.
     * \param[in] sequence_name The name of the sequence results to get. This is
     *    case sensitive.
     * \return A read-only reference to the requested sequence results.
     * \throws invalid_sequence If no sequence exist with name equal to \a
     * sequence_name.
     */
    [[nodiscard]] auto operator[](const std::string& sequence_name) const
      -> const sequence_results&;

    /**
     * \brief Get read-write access to the results for a specific sequence.
     * \param[in] sequence_name The name of the sequence results to get. This is
     *    case sensitive.
     * \return A read-write reference to the requested sequence results.
     * \throws invalid_sequence If no sequence exist with name equal to \a
     * sequence_name.
     */
    [[nodiscard]] auto operator[](const std::string& sequence_name)
      -> sequence_results&;

  private:
    std::string m_name;
    sequence_list m_sequences;
  };

  auto operator==(const tracker_results& tracker,
                  const std::string& name_query) noexcept -> bool;

  /**
   * \brief Get the number of sequences for a tracker.
   * \param[in] tracker Get the number of sequences in this tracker results.
   * \return The number of sequences in the supplied tracker results.
   * \details This behaves as if you called
   * \code {.cpp}
   * tracker.sequences().size();
   * \endcode
   * \related tracker_results
   */
  [[nodiscard]] auto size(const tracker_results& tracker) noexcept
    -> tracker_results::size_type;

  /**
   * \brief Get an iterator to the first sequence results for a tracker.
   * \param[in] tracker Get an iterator for this tracker.
   * \return An iterator to the first sequences results for \a tracker.
   * \details This allows client code to use a tracker_results object in a
   * range-for loop or iterator-style algorithms:
   * \code {.cpp}
   * std::any_of(
   *   analyzer::begin(tracker),
   *   analyzer::end(tracker),
   *   [](const analyzer::sequence_results& sequence) {
   *     return sequence.name() == "Deer";});
   * for (const auto& sequence : tracker) {
   *   // Do something with each sequence results in the tracker results.
   * }
   * \endcode
   * \related tracker_results
   */
  [[nodiscard]] auto begin(const tracker_results& tracker)
    -> tracker_results::sequence_list::const_iterator;

  /**
   * \brief Get a past-the-end iterator to a tracker's sequence results.
   * \param[in] tracker Get an iterator for this tracker.
   * \return A past-the-end iterator to the \a tracker's sequence results.
   * \see begin()
   * \related tracker_results
   */
  [[nodiscard]] auto end(const tracker_results& tracker)
    -> tracker_results::sequence_list::const_iterator;

  /**
   * \brief Encapsulate tracking results from multiple trackers for one
   * benchmark dataset.
   * \details Each record of the database is the tracking results reported by
   * one tracker.
   */
  class results_database final
  {
  public:
    /// The results database is [contiguous
    /// container](https://en.cppreference.com/w/cpp/named_req/ContiguousContainer).
    using tracker_list = std::vector<tracker_results>;
    using size_type = tracker_list::size_type;

    /// Get read-only access to the list of tracker results.
    [[nodiscard]] auto trackers() const noexcept -> const tracker_list&;

    /// Get read-write access to the list of tracker results.
    [[nodiscard]] auto trackers() noexcept -> tracker_list&;

    /**
     * \brief Get read-only access to the results for a specific tracker.
     * \param[in] i The index into the tracker results list. This is a
     *    0-based index like for any other C++ contiguous data structure.
     * \return A read-only reference to the requested tracker results.
     * \throws std::out_of_range If \$i \ge size()\$.
     */
    [[nodiscard]] auto operator[](size_type i) const -> const tracker_results&;

    /**
     * \brief Get read-write access to the results for a specific tracker.
     * \param[in] i The index into the tracker results list. This is a
     *    0-based index like for any other C++ contiguous data structure.
     * \return A read-write reference to the requested tracker results.
     * \throws std::out_of_range If \$i \ge size()\$.
     */
    [[nodiscard]] auto operator[](size_type i) -> tracker_results&;

    /**
     * \brief Get read-only access to the results for a specific tracker.
     * \param[in] tracker_name The name of the tracker results to get. This is
     * case sensitive.
     * \return A read-only reference to the requested tracker results.
     * \throws invalid_tracker If no tracker exist with name equal to \a
     * tracker_name.
     */
    [[nodiscard]] auto operator[](const std::string& tracker_name) const
      -> const tracker_results&;

    /**
     * \brief Get read-write access to the results for a specific tracker.
     * \param[in] tracker_name The name of the tracker results to get. This is
     * case sensitive.
     * \return A read-write reference to the requested tracker results.
     * \throws invalid_tracker If no tracker exist with name equal to \a
     * tracker_name.
     */
    [[nodiscard]] auto operator[](const std::string& tracker_name)
      -> tracker_results&;

  private:
    tracker_list m_trackers;
  };

  /**
   * \brief Get the number of trackers in a database.
   * \param[in] db Get the number of trackers in this results database.
   * \return The number of trackers in the supplied results database.
   * \details This behaves as if you called
   * \code {.cpp}
   * db.trackers().size();
   * \endcode
   * \related results_database
   */
  [[nodiscard]] auto size(const results_database& db) noexcept
    -> results_database::size_type;

  /**
   * \brief Get an iterator to the first tracker results for a results database.
   * \param[in] db Get an iterator for this database.
   * \return An iterator to the first tracker results for \a db.
   * \details This allows client code to use a results_database object in a
   * range-for loop or iterator-style algorithms:
   * \code {.cpp}
   * std::any_of(
   *   analyzer::begin(db),
   *   analyzer::end(db),
   *   [](const analyzer::tracker_results& tracker) {
   *     return tracker.name() == "MDNet";});
   * for (const auto& tracker : db) {
   *   // Do something with each tracker results in the tracker results.
   * }
   * \endcode
   * \related results_database
   */
  [[nodiscard]] auto begin(const results_database& db) noexcept
    -> results_database::tracker_list::const_iterator;

  /**
   * \brief Get a past-the-end iterator for a database's trackers.
   * \param[in] db Get an iterator for this database.
   * \return A past-the-end iterator to the tracker results for \a db.
   * \see begin()
   * \related results_database
   */
  [[nodiscard]] auto end(const results_database& db) noexcept
    -> results_database::tracker_list::const_iterator;

  /**
   * \brief Get a list of all tracker names in a results database.
   * \param[in] db Get the tracker names in this results database.
   * \return A list of just the tracker names in the \a db.
   */
  [[nodiscard]] auto list_all_trackers(const results_database& db)
    -> std::vector<std::string>;

  /**
   * \brief Load results for all trackers found in a directory on disk.
   * \param[in] path The path to the directory to search for tracking results.
   * \return A results_database with the tracking results found in \a path.
   * \details This function searches \a path for tracking results. It assumes
   * that each subdirectory is a tracker. For each subdirectory, it searches
   * for sequence results. It assumes each sequence is a text file in the
   * tracker subdirectory. Here is an example, given \a path is "~/otb_results":
   * \verbatim
   * ~/otb_results/
   *     MDNet/
   *         Basketball.txt
   *         Deer.txt
   *     VITAL/
   *         Basketball.txt
   *         Deer.txt
   * \endverbatim
   * This example includes two trackers: MDNet and VITAL. Each tracker contains
   * results for two sequences: Basketball and Deer.
   */
  [[nodiscard]] auto load_tracking_results_directory(const std::string& path)
    -> results_database;

  void push_front(results_database& db, const tracker_results& tracker);

  auto contains(const results_database& db, const std::string& tracker_name)
    -> bool;
}  // namespace analyzer

#endif
