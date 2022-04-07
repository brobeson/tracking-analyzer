#include "test_utilities.h"
#include "tracking-analyzer/tracking_results.h"
#include <QTest>

using namespace std::literals::string_literals;

namespace analyzer_test
{
  namespace
  {
    // Exceptions during construction of these objects are unlikely. If
    // exceptions are thrown, well, this is a unit test. I'll deal with it.
    // NOLINTNEXTLINE(cert-err58-cpp)
    const analyzer::bounding_box_list expected_boxes {{1.0f, 1.0f, 1.0f, 1.0f},
                                                      {2.0f, 2.0f, 2.0f, 2.0f}};
    // NOLINTNEXTLINE(cert-err58-cpp)
    const analyzer::tracker_results::sequence_list expected_sequences {
      {"Basketball", expected_boxes},
      {"Deer", expected_boxes},
      {"Walking2", expected_boxes}};
    // NOLINTNEXTLINE(cert-err58-cpp)
    const std::string expected_tracker_name {"MDNet"};

    auto make_tracker_results()
    {
      return analyzer::tracker_results {expected_tracker_name,
                                        expected_sequences};
    }
  }  // namespace

  class tracker_results_test final: public QObject
  {
    // NOLINTNEXTLINE(modernize-use-trailing-return-type)
    Q_OBJECT

  private slots:
    void construct_default_tracker_results() const
    {
      const analyzer::tracker_results results;
      QVERIFY(results.name().empty());
      QVERIFY(results.sequences().empty());
    }

    void construct_tracker_results() const
    {
      const analyzer::tracker_results results {
        "MDNet", {analyzer::sequence_results {"Deer", {}}}};
      QCOMPARE(results.name(), "MDNet"s);
      QCOMPARE(results.sequences().size(), 1ul);
    }

    void iteration_test() const
    {
      const auto results {make_tracker_results()};
      analyzer::tracker_results::size_type i {0};
      for (const auto& actual_sequence : results)
      {
        QCOMPARE(actual_sequence.name(), expected_sequences.at(i).name());
        ++i;
      }
    }

    void read_only_sequence_access_test() const
    {
      const auto results {make_tracker_results()};
      QCOMPARE(results.sequences().size(), 3ul);
    }

    void read_write_sequence_access_test() const
    {
      auto results {make_tracker_results()};
      results.sequences().push_back(
        analyzer::sequence_results {"David", {{3.0f, 3.0f, 3.0f, 3.0f}}});
      QCOMPARE(results.sequences().size(), 4ul);
    }

    void read_only_index_test() const
    {
      const auto results {make_tracker_results()};
      QCOMPARE(results[0].name(), expected_sequences.at(0).name());
      QCOMPARE(results[1].name(), expected_sequences.at(1).name());
      QCOMPARE(results[2].name(), expected_sequences.at(2).name());
    }

    void read_only_index_throw_test() const
    {
      const auto results {make_tracker_results()};
      QVERIFY_EXCEPTION_THROWN([[maybe_unused]] const auto b {results[3]},
                               std::out_of_range);
    }

    void read_write_index_test() const
    {
      auto results {make_tracker_results()};
      constexpr analyzer::bounding_box new_box {10.0f, 10.0f, 10.0f, 10.0f};
      results[0][0] = new_box;
      QCOMPARE(results[0][0], new_box);
    }

    void read_write_index_throw_test() const
    {
      auto results {make_tracker_results()};
      constexpr analyzer::bounding_box new_box {10.0f, 10.0f, 10.0f, 10.0f};
      QVERIFY_EXCEPTION_THROWN(results[3][0] = new_box, std::out_of_range);
    }

    void size_test() const
    {
      auto results {make_tracker_results()};
      QCOMPARE(analyzer::size(results), 3ul);
      results.sequences().push_back({"David", expected_boxes});
      QCOMPARE(analyzer::size(results), 4ul);
    }

    void read_only_sequence_search_test() const
    {
      const auto results {make_tracker_results()};
      QCOMPARE(results["Deer"].name(), "Deer"s);
    }

    void read_only_sequence_search_throw_test() const
    {
      const auto results {make_tracker_results()};
      QVERIFY_EXCEPTION_THROWN(
        [[maybe_unused]] const auto s {results["deer"].name()},
        analyzer::invalid_sequence);
    }

    void read_write_sequence_search_test() const
    {
      auto results {make_tracker_results()};
      QCOMPARE(results["Deer"].name(), "Deer"s);
    }

    void read_write_sequence_search_throw_test() const
    {
      auto results {make_tracker_results()};
      QVERIFY_EXCEPTION_THROWN([[maybe_unused]] auto s {results["deer"].name()},
                               analyzer::invalid_sequence);
    }

    void equality_comparison_test() const
    {
      const analyzer::tracker_results results {"MDNet", {}};
      QVERIFY(results == "MDNet");
      QVERIFY(!(results == "mdnet"));
    }
  };
}  // namespace analyzer_test

// NOLINTNEXTLINE(modernize-use-trailing-return-type)
QTEST_APPLESS_MAIN(analyzer_test::tracker_results_test)
#include "tracker_results_test.moc"
