#include "bounding_box.h"
#include <QTest>
#include <sstream>

Q_DECLARE_METATYPE(std::string)
Q_DECLARE_METATYPE(analyzer::bounding_box)
Q_DECLARE_METATYPE(analyzer::bounding_box_list)

namespace analyzer
{
  // This operator must not be in an unnamed namespace, or the compiler can't
  // find it for later operations. Also, I know comparing floating point data
  // like this is generally unsafe, but it works for this unit test.
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-declarations"
#pragma GCC diagnostic ignored "-Wfloat-equal"
  [[nodiscard]] bool operator==(const analyzer::bounding_box& a,
                                const analyzer::bounding_box& b)
  {
    return a.x == b.x && a.y == b.y && a.width == b.width
           && a.height == b.height;
  }
#pragma GCC diagnostic pop
}  // namespace analyzer

namespace analyzer_test
{
  using namespace std::literals::string_literals;

  namespace
  {
    void create_list_data()
    {
      QTest::addColumn<analyzer::bounding_box_list>("a");
      QTest::addColumn<analyzer::bounding_box_list>("b");
      QTest::addColumn<analyzer::overlap_list>("overlaps");
      QTest::addColumn<analyzer::offset_list>("offsets");
      QTest::newRow("empty list")
        << analyzer::bounding_box_list {} << analyzer::bounding_box_list {}
        << analyzer::overlap_list {} << analyzer::offset_list {};
      QTest::newRow("a list")
        << analyzer::bounding_box_list {{0.0f, 0.0f, 0.0f, 0.0f},
                                        {0.0f, 0.0f, 10.0f, 10.0f}}
        << analyzer::bounding_box_list {{0.0f, 0.0f, 100.0f, 100.0f},
                                        {0.0f, 0.0f, 5.0f, 10.0f}}
        << analyzer::overlap_list {0.0f, 0.5f}
        << analyzer::offset_list {70.71067811865476f, 2.5f};
    }
  }  // namespace

  class bounding_box_test final: public QObject
  {
    Q_OBJECT

  private slots:
    void read_valid_bounding_boxes_data() const
    {
      QTest::addColumn<std::string>("data");
      QTest::addColumn<analyzer::bounding_box_list>("expected_output");
      QTest::newRow("empty stream") << ""s << analyzer::bounding_box_list {};
      QTest::newRow("singe box")
        << "0,0,100,73"s
        << analyzer::bounding_box_list {{0.0f, 0.0f, 100.0f, 73.0f}};
      QTest::newRow("four boxes")
        << "0,0,100,73\n1,1,101,70\n10,11,110,80\n78,42,100,70"s
        << analyzer::bounding_box_list {{0.0f, 0.0f, 100.0f, 73.0f},
                                        {1.0f, 1.0f, 101.0f, 70.0f},
                                        {10.0f, 11.0f, 110.0f, 80.0f},
                                        {78.0f, 42.0f, 100.0f, 70.0f}};
      QTest::newRow("floating point boxes")
        << "0.0,0.0,100.3456,73.0540"s
        << analyzer::bounding_box_list {{0.0f, 0.0f, 100.3456f, 73.054f}};
    }

    void read_valid_bounding_boxes() const
    {
      QFETCH(const std::string, data);
      QFETCH(const analyzer::bounding_box_list, expected_output);
      std::istringstream stream {data};
      const auto actual_output {analyzer::read_bounding_boxes(stream)};
      QCOMPARE(actual_output, expected_output);
    }

    void read_invalid_bounding_boxes_data() const
    {
      QTest::addColumn<std::string>("data");
      QTest::newRow("three values") << "0,0,0"s;
      QTest::newRow("five values") << "0,0,0,0,0"s;
      QTest::newRow("one invalid row") << "0,0,10,10\n10,11,9,8\n10,11,9"s;
    }

    void read_invalid_bounding_boxes() const
    {
      QFETCH(const std::string, data);
      std::istringstream stream {data};
      QVERIFY_EXCEPTION_THROWN(analyzer::read_bounding_boxes(stream),
                               analyzer::invalid_data);
    }

    void calculate_overlaps_data() const { analyzer_test::create_list_data(); }

    void calculate_overlaps() const
    {
      QFETCH(const analyzer::bounding_box_list, a);
      QFETCH(const analyzer::bounding_box_list, b);
      QTEST(analyzer::calculate_overlaps(a, b), "overlaps");
    }

    void calculate_offsets_data() const { analyzer_test::create_list_data(); }

    void calculate_offsets() const
    {
      QFETCH(const analyzer::bounding_box_list, a);
      QFETCH(const analyzer::bounding_box_list, b);
      QTEST(analyzer::calculate_offsets(a, b), "offsets");
    }

    void calculate_overlap_data() const
    {
      QTest::addColumn<analyzer::bounding_box>("a");
      QTest::addColumn<analyzer::bounding_box>("b");
      QTest::addColumn<analyzer::offset>("expected_overlap");
      QTest::newRow("a left of b")
        << analyzer::bounding_box {3.0f, 5.0f, 20.0f, 30.0f}
        << analyzer::bounding_box {25.0f, 5.0f, 20.0f, 30.0f} << 0.0f;
      QTest::newRow("a above b")
        << analyzer::bounding_box {3.0f, 5.0f, 20.0f, 30.0f}
        << analyzer::bounding_box {3.0f, 35.0f, 20.0f, 30.0f} << 0.0f;
      QTest::newRow("a right of b")
        << analyzer::bounding_box {25.0f, 5.0f, 20.0f, 30.0f}
        << analyzer::bounding_box {3.0f, 5.0f, 20.0f, 30.0f} << 0.0f;
      QTest::newRow("a below b")
        << analyzer::bounding_box {3.0f, 35.0f, 20.0f, 30.0f}
        << analyzer::bounding_box {3.0f, 5.0f, 20.0f, 30.0f} << 0.0f;
      QTest::newRow("coincident boxes")
        << analyzer::bounding_box {3.0f, 35.0f, 20.0f, 30.0f}
        << analyzer::bounding_box {3.0f, 35.0f, 20.0f, 30.0f} << 1.0f;
      QTest::newRow("half overlap")
        << analyzer::bounding_box {3.0f, 5.0f, 20.0f, 30.0f}
        << analyzer::bounding_box {13.0f, 5.0f, 10.0f, 30.0f} << 0.5f;
      QTest::newRow("one seventh overlap")
        << analyzer::bounding_box {3.0f, 5.0f, 20.0f, 30.0f}
        << analyzer::bounding_box {13.0f, 20.0f, 20.0f, 30.0f} << 1.0f / 7.0f;
      QTest::newRow("zero overlap")
        << analyzer::bounding_box {3.0f, 5.0f, 20.0f, 30.0f}
        << analyzer::bounding_box {13.0f, 20.0f, 0.0f, 0.0f} << 0.0f;
    }

    void calculate_overlap() const
    {
      QFETCH(const analyzer::bounding_box, a);
      QFETCH(const analyzer::bounding_box, b);
      QTEST(analyzer::calculate_overlap(a, b), "expected_overlap");
    }

    void calculate_offset_data() const
    {
      QTest::addColumn<analyzer::bounding_box>("a");
      QTest::addColumn<analyzer::bounding_box>("b");
      QTest::addColumn<analyzer::offset>("expected_offset");
      QTest::newRow("a left of b")
        << analyzer::bounding_box {3.0f, 5.0f, 20.0f, 30.0f}
        << analyzer::bounding_box {25.0f, 5.0f, 20.0f, 30.0f} << 22.0f;
      QTest::newRow("a above b")
        << analyzer::bounding_box {3.0f, 5.0f, 20.0f, 30.0f}
        << analyzer::bounding_box {3.0f, 35.0f, 20.0f, 30.0f} << 30.0f;
      QTest::newRow("a right of b")
        << analyzer::bounding_box {25.0f, 5.0f, 20.0f, 30.0f}
        << analyzer::bounding_box {3.0f, 5.0f, 20.0f, 30.0f} << 22.0f;
      QTest::newRow("a below b")
        << analyzer::bounding_box {3.0f, 35.0f, 20.0f, 30.0f}
        << analyzer::bounding_box {3.0f, 5.0f, 20.0f, 30.0f} << 30.0f;
      QTest::newRow("coincident boxes")
        << analyzer::bounding_box {3.0f, 35.0f, 20.0f, 30.0f}
        << analyzer::bounding_box {8.0f, 45.0f, 10.0f, 10.0f} << 0.0f;
      QTest::newRow("one seventh overlap")
        << analyzer::bounding_box {3.0f, 5.0f, 20.0f, 30.0f}
        << analyzer::bounding_box {13.0f, 20.0f, 34.0f, 29.0f}
        << 22.34390297150433f;
    }

    void calculate_offset() const
    {
      QFETCH(const analyzer::bounding_box, a);
      QFETCH(const analyzer::bounding_box, b);
      QTEST(analyzer::calculate_offset(a, b), "expected_offset");
    }
  };
}  // namespace analyzer_test

QTEST_APPLESS_MAIN(analyzer_test::bounding_box_test)
#include "bounding_box_test.moc"