#include "tracking-analyzer/training_metadata.h"
#include <QTest>

Q_DECLARE_METATYPE(analyzer::training_batch)   // NOLINT
Q_DECLARE_METATYPE(analyzer::training_scores)  // NOLINT

namespace analyzer_test
{
  namespace
  {
    void compare(const analyzer::training_batch& actual,
                 const analyzer::training_batch& expected)
    {
      QCOMPARE(actual.background_threshold, expected.background_threshold);
      QCOMPARE(actual.target_threshold, expected.target_threshold);
      QCOMPARE(actual.background_candidates, expected.background_candidates);
      QCOMPARE(actual.background_mined, expected.background_mined);
      QCOMPARE(actual.target_candidates, expected.target_candidates);
    }

    void compare(const analyzer::training_update& actual,
                 const analyzer::training_update& expected)
    {
      QCOMPARE(actual.size(), expected.size());
      for (analyzer::training_update::size_type i {0}; i < actual.size(); ++i)
      {
        compare(actual.at(i), expected.at(i));
      }
    }

    void compare(const analyzer::update_list& actual,
                 const analyzer::update_list& expected)
    {
      QCOMPARE(actual.size(), expected.size());
      for (analyzer::update_list::size_type i {0}; i < actual.size(); ++i)
      {
        compare(actual.at(i), expected.at(i));
      }
    }
  }  // namespace

  class training_metadata_test final: public QObject
  {
    // NOLINTNEXTLINE(modernize-use-trailing-return-type)
    Q_OBJECT

  private slots:
    void load_training_scores_throws_data() const
    {
      QTest::addColumn<QString>("path");
      QTest::newRow("empty path") << "";
      QTest::newRow("file does not exist") << "no_such_file.json";
      QTest::newRow("invalid JSON") << "invalid_training_metadata.json";
    }

    void load_training_scores_throws() const
    {
      QFETCH(const QString, path);
      QVERIFY_EXCEPTION_THROWN(
        const auto scores {analyzer::load_training_scores(path)},
        std::exception);
    }

    void load_training_scores_data() const
    {
      // #lizard forgives
      // All the test data makes this function long, but there isn't anything
      // to be done about it.
      QTest::addColumn<QString>("path");
      QTest::addColumn<analyzer::training_scores>("expected_scores");
      QTest::newRow("full data")
        << "full.json"
        << analyzer::training_scores {
             "Deer",
             "OTB-100",
             std::vector<int> {0, 20},
             analyzer::update_list {
               {analyzer::training_batch {
                 analyzer::score_list {QPointF {13.0f, -11.0f},
                                       QPointF {13.0f, -12.0f},
                                       QPointF {20.0f, -18.0f},
                                       QPointF {12.0f, -11.0f},
                                       QPointF {116.0f, -103.0f}},
                 analyzer::score_list {QPointF {12.0f, -11.0f},
                                       QPointF {13.0f, -11.0f},
                                       QPointF {13.0f, -12.0f}},
                 analyzer::score_list {QPointF {-5.0f, 5.0f},
                                       QPointF {-7.0f, 8.0f},
                                       QPointF {-9.0f, 9.0f},
                                       QPointF {-4.0f, 4.0f},
                                       QPointF {-9.0f, 10.0f},
                                       QPointF {-2.0f, 2.0f}},
                 20.0f,
                 -103.0f}},
               {analyzer::training_batch {
                 analyzer::score_list {QPointF {54.0f, -48.0f},
                                       QPointF {122.0f, -109.0f},
                                       QPointF {17.0f, -15.0f},
                                       QPointF {42.0f, -37.0f},
                                       QPointF {29.0f, -26.0f}},
                 analyzer::score_list {QPointF {17.0f, -15.0f},
                                       QPointF {29.0f, -26.0f},
                                       QPointF {42.0f, -37.0f}},
                 analyzer::score_list {QPointF {-10.0f, 9.0f},
                                       QPointF {-6.0f, 9.0f},
                                       QPointF {-3.0f, 4.0f},
                                       QPointF {-7.0f, 6.0f},
                                       QPointF {-7.0f, 7.0f},
                                       QPointF {-9.0f, 10.0f}},
                 54.0f,
                 -109.0f}}

             }};
      QTest::newRow("bg candidates")
        << "bg_candidates.json"
        << analyzer::training_scores {
             "Deer",
             "OTB-100",
             std::vector<int> {0, 20},
             analyzer::update_list {
               {analyzer::training_batch {
                 analyzer::score_list {QPointF {13.0f, -11.0f},
                                       QPointF {13.0f, -12.0f},
                                       QPointF {20.0f, -18.0f},
                                       QPointF {12.0f, -11.0f},
                                       QPointF {116.0f, -103.0f}},
                 analyzer::score_list {},
                 analyzer::score_list {},
                 20.0f,
                 -103.0f}},
               {analyzer::training_batch {
                 analyzer::score_list {QPointF {54.0f, -48.0f},
                                       QPointF {122.0f, -109.0f},
                                       QPointF {17.0f, -15.0f},
                                       QPointF {42.0f, -37.0f},
                                       QPointF {29.0f, -26.0f}},
                 analyzer::score_list {},
                 analyzer::score_list {},
                 54.0f,
                 -109.0f}}

             }};
      QTest::newRow("bg mined")
        << "bg_mined.json"
        << analyzer::training_scores {
             "Deer",
             "OTB-100",
             std::vector<int> {0, 20},
             analyzer::update_list {
               {analyzer::training_batch {
                 analyzer::score_list {},
                 analyzer::score_list {QPointF {12.0f, -11.0f},
                                       QPointF {13.0f, -11.0f},
                                       QPointF {13.0f, -12.0f}},
                 analyzer::score_list {},
                 20.0f,
                 -103.0f}},
               {analyzer::training_batch {
                 analyzer::score_list {},
                 analyzer::score_list {QPointF {17.0f, -15.0f},
                                       QPointF {29.0f, -26.0f},
                                       QPointF {42.0f, -37.0f}},
                 analyzer::score_list {},
                 54.0f,
                 -109.0f}}

             }};
      QTest::newRow("tg candidates")
        << "tg_candidates.json"
        << analyzer::training_scores {
             "Deer",
             "OTB-100",
             std::vector<int> {0, 20},
             analyzer::update_list {
               {analyzer::training_batch {
                 analyzer::score_list {},
                 analyzer::score_list {},
                 analyzer::score_list {QPointF {-5.0f, 5.0f},
                                       QPointF {-7.0f, 8.0f},
                                       QPointF {-9.0f, 9.0f},
                                       QPointF {-4.0f, 4.0f},
                                       QPointF {-9.0f, 10.0f},
                                       QPointF {-2.0f, 2.0f}},
                 20.0f,
                 -103.0f}},
               {analyzer::training_batch {
                 analyzer::score_list {},
                 analyzer::score_list {},
                 analyzer::score_list {QPointF {-10.0f, 9.0f},
                                       QPointF {-6.0f, 9.0f},
                                       QPointF {-3.0f, 4.0f},
                                       QPointF {-7.0f, 6.0f},
                                       QPointF {-7.0f, 7.0f},
                                       QPointF {-9.0f, 10.0f}},
                 54.0f,
                 -109.0f}}

             }};
      QTest::newRow("thresholds")
        << "thresholds.json"
        << analyzer::training_scores {
             "Deer",
             "OTB-100",
             std::vector<int> {0, 20},
             analyzer::update_list {
               {analyzer::training_batch {analyzer::score_list {},
                                          analyzer::score_list {},
                                          analyzer::score_list {},
                                          20.0f,
                                          -103.0f}},
               {analyzer::training_batch {analyzer::score_list {},
                                          analyzer::score_list {},
                                          analyzer::score_list {},
                                          54.0f,
                                          -109.0f}}

             }};
    }

    void load_training_scores() const
    {
      QFETCH(const QString, path);
      const auto actual_scores {analyzer::load_training_scores(
        "test_metadata/training_metadata/" + path)};
      QFETCH(const analyzer::training_scores, expected_scores);
      QCOMPARE(actual_scores.sequence_name, expected_scores.sequence_name);
      QCOMPARE(actual_scores.dataset, expected_scores.dataset);
      compare(actual_scores.updates, expected_scores.updates);
    }

    void get_chart_range_data() const
    {
      QTest::addColumn<analyzer::training_batch>("batch");
      QTest::addColumn<analyzer::range>("expected_range");
      QTest::newRow("empty batch")
        << analyzer::training_batch {} << analyzer::range {0.0f, 0.0f};
      QTest::newRow("only bg candidates") << analyzer::training_batch {
        analyzer::score_list {{-20.0f, 0.0f}, {0.0f, 20.0f}},
        analyzer::score_list {},
        analyzer::score_list {},
        0.0f,
        0.0f} << analyzer::range {-20.0f, 20.0f};
      QTest::newRow("only bg mined") << analyzer::training_batch {
        analyzer::score_list {},
        analyzer::score_list {{-20.0f, 0.0f}, {0.0f, 20.0f}},
        analyzer::score_list {},
        0.0f,
        0.0f} << analyzer::range {-20.0f, 20.0f};
      QTest::newRow("only tg candidates") << analyzer::training_batch {
        analyzer::score_list {},
        analyzer::score_list {},
        analyzer::score_list {{-20.0f, 0.0f}, {0.0f, 20.0f}},
        0.0f,
        0.0f} << analyzer::range {-20.0f, 20.0f};
      QTest::newRow("only thresholds")
        << analyzer::training_batch {analyzer::score_list {},
                                     analyzer::score_list {},
                                     analyzer::score_list {},
                                     20.0f,
                                     -20.0f}
        << analyzer::range {-20.0f, 20.0f};
    }

    void get_chart_range() const
    {
      QFETCH(const analyzer::training_batch, batch);
      QTEST(analyzer::get_chart_range(batch), "expected_range");
    }
  };
}  // namespace analyzer_test

// NOLINTNEXTLINE(modernize-use-trailing-return-type)
QTEST_APPLESS_MAIN(analyzer_test::training_metadata_test)
#include "training_metadata_test.moc"
