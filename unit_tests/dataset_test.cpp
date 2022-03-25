#include "tracking-analyzer/dataset.h"
#include <QDebug>
#include <QTest>
#include <iostream>

Q_DECLARE_METATYPE(analyzer::dataset)   // NOLINT
Q_DECLARE_METATYPE(analyzer::sequence)  // NOLINT

using namespace std::literals::string_literals;

namespace analyzer
{
  // This operator must not be in an unnamed namespace, or the compiler can't
  // find it for later operations.
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-declarations"
  [[nodiscard]] auto operator==(const analyzer::dataset& a,
                                const analyzer::dataset& b)
  {
    return a.root_path() == b.root_path() && a.sequences() == b.sequences();
  }

  [[nodiscard]] auto operator==(const analyzer::frame_record& a,
                                const analyzer::frame_record& b)
  {
    return a.image_path() == b.image_path();
  }

  [[nodiscard]] auto operator==(const analyzer::sequence& a,
                                const analyzer::sequence& b)
  {
    return a.name() == b.name() && a.frames() == b.frames();
  }
#pragma GCC diagnostic pop
}  // namespace analyzer

namespace analyzer_test
{
  using namespace std::literals::string_literals;

  class dataset_test final: public QObject
  {
    // NOLINTNEXTLINE(modernize-use-trailing-return-type)
    Q_OBJECT

  private slots:
    void frame_record_test() const
    {
      const analyzer::frame_record a;
      QVERIFY(a.image_path().empty());
      const analyzer::frame_record b {"a path"};
      QCOMPARE(b.image_path(), "a path"s);
    }

    //--------------------------------------------------------------------------
    //                                                old code - not refactored
    //          yet see https://github.com/brobeson/tracking-analyzer/issues/41
    //--------------------------------------------------------------------------
    void construct_default_dataset() const
    {
      const analyzer::dataset d;
      QVERIFY(d.sequences().isEmpty());
      QVERIFY(d.root_path().isEmpty());
    }

    void load_dataset_data() const
    {
      QTest::addColumn<QString>("path");
      QTest::addColumn<analyzer::dataset>("dataset");
      // QTest::newRow("nonexistent directory")
      //   << "ghost/"
      //   << analyzer::dataset {"ghost/", QVector<analyzer::sequence> {}};
      // QTest::newRow("empty directory")
      //   << "empty_dataset/"
      //   << analyzer::dataset {"empty_dataset/", QVector<analyzer::sequence>
      //   {}};
      QTest::newRow("partial OTB")
        << "test_dataset"
        << analyzer::dataset {
             "test_dataset",
             QVector {
               analyzer::sequence_record {
                 "Basketball", "test_dataset/Basketball", {}, {}},
               analyzer::sequence_record {
                 "Biker",
                 "test_dataset/Biker",
                 {},
                 analyzer::sequence_record::frame_list {
                   analyzer::frame_record {"test_dataset/Biker/img/0001.jpg"},
                   analyzer::frame_record {"test_dataset/Biker/img/0002.jpg"},
                   analyzer::frame_record {"test_dataset/Biker/img/0003.jpg"}}},
               analyzer::sequence_record {
                 "Bird1", "test_dataset/Bird1", {}, {}},
               analyzer::sequence_record {
                 "Bird2", "test_dataset/Bird2", {}, {}},
             }};
    }

    void load_dataset() const
    {
      QFETCH(const QString, path);
      // const auto ds {analyzer::load_dataset(path)};
      // QFETCH(const analyzer::dataset, dataset);
      // QCOMPARE(ds.sequences().size(), dataset.sequences().size());
      // for (int i {0}; i < ds.sequences().size(); ++i)
      // {
      //   QCOMPARE(ds.sequences()[i].name(), dataset.sequences()[i].name());
      //   QCOMPARE(ds.sequences()[i].frames().size(),
      //            dataset.sequences()[i].frames().size());
      // }
      // QCOMPARE(ds.root_path(), QString("test_dataset"));
      QTEST(analyzer::load_dataset(path), "dataset");
    }

    void sequence_names_data() const
    {
      using sequence_list = QVector<analyzer::sequence>;
      QTest::addColumn<sequence_list>("sequences");
      QTest::addColumn<QStringList>("expected_names");
      QTest::newRow("empty list") << sequence_list {} << QStringList {};
      QTest::newRow("one sequence")
        << sequence_list {{"Biker", "test_dataset/Biker", {}, {}}}
        << QStringList {"Biker"};
      QTest::newRow("two sequences")
        << sequence_list {{"Biker", "test_dataset/Biker", {}, {}},
                          {"Dancer", "test_dataset/Biker", {}, {}}}
        << QStringList {"Biker", "Dancer"};
    }

    void sequence_names() const
    {
      QFETCH(const QVector<analyzer::sequence>, sequences);
      QTEST(analyzer::sequence_names(sequences), "expected_names");
    }
  };
}  // namespace analyzer_test

// NOLINTNEXTLINE(modernize-use-trailing-return-type)
QTEST_APPLESS_MAIN(analyzer_test::dataset_test)
#include "dataset_test.moc"