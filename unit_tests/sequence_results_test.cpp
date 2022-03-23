#include "test_utilities.h"
#include "tracking-analyzer/tracking_results.h"
#include <QTest>

using namespace std::literals::string_literals;

namespace analyzer_test
{
  class sequence_results_test final: public QObject
  {
    // NOLINTNEXTLINE(modernize-use-trailing-return-type)
    Q_OBJECT

  private slots:
    void construct_default_sequence_results() const
    {
      const analyzer::sequence_results results;
      QVERIFY(results.name().empty());
      QVERIFY(results.bounding_boxes().empty());
    }

    void construct_sequence_results() const
    {
      constexpr analyzer::bounding_box expected_box {1.0f, 1.0f, 1.0f, 1.0f};
      const analyzer::sequence_results results {"Deer", {expected_box}};
      QCOMPARE(results.name(), "Deer"s);
      QCOMPARE(results.bounding_boxes().size(), 1ul);
      QCOMPARE(results.bounding_boxes().front(), expected_box);
    }

    void iteration_test() const
    {
      const analyzer::bounding_box_list expected_boxes {
        {1.0f, 1.0f, 1.0f, 1.0f}, {2.0f, 2.0f, 2.0f, 2.0f}};
      const analyzer::sequence_results results {"Deer", expected_boxes};
      analyzer::bounding_box_list::size_type i {0};
      for (const auto& actual_box : results)
      {
        QCOMPARE(actual_box, expected_boxes.at(i));
        ++i;
      }
    }

    void read_only_box_access_test() const
    {
      const analyzer::bounding_box_list expected_boxes {
        {1.0f, 1.0f, 1.0f, 1.0f}, {2.0f, 2.0f, 2.0f, 2.0f}};
      const analyzer::sequence_results results {"Deer", expected_boxes};
      QCOMPARE(results.bounding_boxes().size(), 2ul);
    }

    void read_write_box_access_test() const
    {
      const analyzer::bounding_box_list expected_boxes {
        {1.0f, 1.0f, 1.0f, 1.0f}, {2.0f, 2.0f, 2.0f, 2.0f}};
      analyzer::sequence_results results {"Deer", expected_boxes};
      results.bounding_boxes().push_back({3.0f, 3.0f, 3.0f, 3.0f});
      QCOMPARE(results.bounding_boxes().size(), 3ul);
    }

    void read_only_index_test() const
    {
      const analyzer::bounding_box_list expected_boxes {
        {1.0f, 1.0f, 1.0f, 1.0f}, {2.0f, 2.0f, 2.0f, 2.0f}};
      const analyzer::sequence_results results {"Deer", expected_boxes};
      QCOMPARE(results[0], expected_boxes.at(0));
      QCOMPARE(results[1], expected_boxes.at(1));
    }

    void read_only_index_throw_test() const
    {
      const analyzer::bounding_box_list expected_boxes {
        {1.0f, 1.0f, 1.0f, 1.0f}, {2.0f, 2.0f, 2.0f, 2.0f}};
      const analyzer::sequence_results results {"Deer", expected_boxes};
      QVERIFY_EXCEPTION_THROWN([[maybe_unused]] const auto b {results[2]},
                               std::out_of_range);
    }

    void read_write_index_test() const
    {
      const analyzer::bounding_box_list expected_boxes {
        {10.0f, 10.0f, 10.0f, 10.0f}, {20.0f, 20.0f, 20.0f, 20.0f}};
      analyzer::sequence_results results {
        "Deer", {{1.0f, 1.0f, 1.0f, 1.0f}, {2.0f, 2.0f, 2.0f, 2.0f}}};
      results[0] = analyzer::bounding_box {10.0f, 10.0f, 10.0f, 10.0f};
      results[1] = analyzer::bounding_box {20.0f, 20.0f, 20.0f, 20.0f};
      QCOMPARE(results[0], expected_boxes.at(0));
      QCOMPARE(results[1], expected_boxes.at(1));
    }

    void read_write_index_throw_test() const
    {
      analyzer::sequence_results results {
        "Deer", {{1.0f, 1.0f, 1.0f, 1.0f}, {2.0f, 2.0f, 2.0f, 2.0f}}};
      constexpr analyzer::bounding_box new_box {10.0f, 10.0f, 10.0f, 10.0f};
      QVERIFY_EXCEPTION_THROWN(results[2] = new_box, std::out_of_range);
    }

    void size_test() const
    {
      analyzer::sequence_results results {"Deer", {}};
      QCOMPARE(analyzer::size(results), 0ul);
      results.bounding_boxes().push_back({1.0f, 1.0f, 1.0f, 1.0f});
      QCOMPARE(analyzer::size(results), 1ul);
    }
  };
}  // namespace analyzer_test

// NOLINTNEXTLINE(modernize-use-trailing-return-type)
QTEST_APPLESS_MAIN(analyzer_test::sequence_results_test)
#include "sequence_results_test.moc"
