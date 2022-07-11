#include "tracking-analyzer/dataset.h"
#include <QDebug>
#include <QTest>

// Q_DECLARE_METATYPE(analyzer::dataset)   // NOLINT
// Q_DECLARE_METATYPE(analyzer::sequence)  // NOLINT

// namespace analyzer
// {
//   // This operator must not be in an unnamed namespace, or the compiler can't
//   // find it for later operations.
// #pragma GCC diagnostic push
// #pragma GCC diagnostic ignored "-Wmissing-declarations"
//   [[nodiscard]] auto operator==(const analyzer::dataset& a,
//                                 const analyzer::dataset& b)
//   {
//     return a.root_path() == b.root_path() && a.sequences() == b.sequences();
//   }

//   [[nodiscard]] auto operator==(const analyzer::sequence& a,
//                                 const analyzer::sequence& b)
//   {
//     return a.name() == b.name() && a.frame_paths() == b.frame_paths();
//   }
// #pragma GCC diagnostic pop
// }  // namespace analyzer

namespace analyzer_test
{
  using namespace std::literals::string_literals;

  class sequence_record_test final: public QObject
  {
    // NOLINTNEXTLINE(modernize-use-trailing-return-type)
    Q_OBJECT

  private slots:

    void construct_default_sequence_record() const
    {
      const analyzer::sequence_record sequence;
      QVERIFY(sequence.name().empty());
      QVERIFY(sequence.root_path().empty());
      QVERIFY(sequence.challenge_tags().empty());
      QVERIFY(sequence.frames().empty());
    }

    void construct_sequence_record() const
    {
      const analyzer::sequence_record sequence {
        "name",
        "path",
        {"occlusion", "rotation"},
        {analyzer::frame_record {"path/1.jpg"},
         analyzer::frame_record {"path/2.jpg"},
         analyzer::frame_record {"path/3.jpg"}}};
      QCOMPARE(sequence.name(), "name"s);
      QCOMPARE(sequence.root_path(), "path"s);
      QCOMPARE(sequence.challenge_tags().size(), 2ul);
      QCOMPARE(sequence.frames().size(), 3ul);
    }

    void iterate_over_frames() const
    {
      const analyzer::sequence_record sequence {
        "",
        "",
        {},
        {analyzer::frame_record {"1.jpg"},
         analyzer::frame_record {"2.jpg"},
         analyzer::frame_record {"3.jpg"}}};
      const std::array<std::string, 3> expected_paths {
        {"1.jpg", "2.jpg", "3.jpg"}};
      uint64_t i {0};
      for (const auto& frame : sequence)
      {
        QCOMPARE(frame.image_path(), expected_paths.at(i));
        ++i;
      }
    }
  };
}  // namespace analyzer_test

// NOLINTNEXTLINE(modernize-use-trailing-return-type)
QTEST_APPLESS_MAIN(analyzer_test::sequence_record_test)
#include "sequence_record_test.moc"