#include "tracking-analyzer/dataset.h"
#include <QDebug>
#include <QTest>

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

  [[nodiscard]] auto operator==(const analyzer::sequence& a,
                                const analyzer::sequence& b)
  {
    return a.name() == b.name() && a.frame_paths() == b.frame_paths();
  }
#pragma GCC diagnostic pop
}  // namespace analyzer

namespace analyzer_test
{
  class dataset_test final: public QObject
  {
    // NOLINTNEXTLINE(modernize-use-trailing-return-type)
    Q_OBJECT

  private slots:
    void construct_invalid_sequence_exception() const
    {
      const analyzer::invalid_sequence e {"name", "path", "what"};
      QCOMPARE(e.name(), "name"s);
      QCOMPARE(e.path(), "path"s);
      QCOMPARE(e.what(), "what");
    }

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
      QTest::newRow("nonexistent directory")
        << "ghost/"
        << analyzer::dataset {"ghost/", QVector<analyzer::sequence> {}};
      QTest::newRow("empty directory")
        << "empty_dataset/"
        << analyzer::dataset {"empty_dataset/", QVector<analyzer::sequence> {}};
      QTest::newRow("partial OTB")
        << "test_dataset"
        << analyzer::dataset {"test_dataset",
                              {{"Biker", "test_dataset/Biker"}}};
    }

    void load_dataset() const
    {
      QFETCH(const QString, path);
      QTEST(analyzer::load_dataset(path), "dataset");
    }

    void construct_invalid_sequence_data() const
    {
      QTest::addColumn<QString>("path");
      QTest::newRow("empty path") << "";
      QTest::newRow("path doesn't exist") << "/opt/otb/Basketball";
      QTest::newRow("path has no data") << "test_dataset/Basketball";
    }

    void construct_invalid_sequence() const
    {
      QFETCH(const QString, path);
      QVERIFY_EXCEPTION_THROWN(analyzer::sequence("Basketball", path),
                               analyzer::invalid_sequence);
    }

    void construct_default_sequence() const
    {
      const analyzer::sequence s;
      QVERIFY(s.name().isEmpty());
      QVERIFY(s.frame_paths().isEmpty());
      QVERIFY(s.path().isEmpty());
      QVERIFY(s.target_boxes().empty());
    }

    void construct_sequence() const
    {
      const analyzer::sequence biker {"Biker", "test_dataset/Biker"};
      const auto current_working_directory {QDir::currentPath()};
      QCOMPARE(biker.name(), QString {"Biker"});
      const QStringList expected_paths {
        current_working_directory + "/test_dataset/Biker/img/0001.jpg",
        current_working_directory + "/test_dataset/Biker/img/0002.jpg",
        current_working_directory + "/test_dataset/Biker/img/0003.jpg"};
      QCOMPARE(biker.frame_paths(), expected_paths);
    }

    void sequence_names_data() const
    {
      using sequence_list = QVector<analyzer::sequence>;
      QTest::addColumn<sequence_list>("sequences");
      QTest::addColumn<QStringList>("expected_names");
      QTest::newRow("empty list") << sequence_list {} << QStringList {};
      QTest::newRow("one sequence")
        << sequence_list {{"Biker", "test_dataset/Biker"}}
        << QStringList {"Biker"};
      QTest::newRow("two sequences")
        << sequence_list {{"Biker", "test_dataset/Biker"},
                          {"Dancer", "test_dataset/Biker"}}
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