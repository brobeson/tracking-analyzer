#include "tracking-analyzer/dataset.h"
#include <QDebug>
#include <QTest>

Q_DECLARE_METATYPE(analyzer::dataset)  // NOLINT

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
    Q_OBJECT  // NOLINT

      // clang-format off
  private slots:
    // NOLINTNEXTLINE(readability-convert-member-functions-to-static)
    void make_absolute_path_data() const
    // clang-format on
    {
      QTest::addColumn<QString>("path");
      QTest::addColumn<QString>("absolute_path");
      QTest::newRow("Linux relative path") << "Videos/"
                                           << "Videos/";
      QTest::newRow("Linux with tilde") << "~/Videos/"
                                        << "/home/user/Videos/";
      QTest::newRow("Linux absolute path") << "/home/user/Videos/"
                                           << "/home/user/Videos/";
      QTest::newRow("Windows relative path") << "\\Videos\\"
                                             << "\\Videos\\";
      QTest::newRow("Windows path") << "C:\\Users\\user\\Videos\\"
                                    << "C:\\Users\\user\\Videos\\";
    }

    // NOLINTNEXTLINE(readability-convert-member-functions-to-static)
    void make_absolute_path() const
    {
      QFETCH(const QString, path);
      QTEST(analyzer::make_absolute_path(path), "absolute_path");
    }

    // NOLINTNEXTLINE(readability-convert-member-functions-to-static)
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

    // NOLINTNEXTLINE(readability-convert-member-functions-to-static)
    void load_dataset() const
    {
      QFETCH(const QString, path);
      QTEST(analyzer::load_dataset(path), "dataset");
    }

    // NOLINTNEXTLINE(readability-convert-member-functions-to-static)
    void construct_invalid_sequence_data() const
    {
      QTest::addColumn<QString>("path");
      QTest::newRow("empty path") << "";
      QTest::newRow("path doesn't exist") << "/opt/otb/Basketball";
      QTest::newRow("path has no data") << "test_dataset/Basketball";
    }

    // NOLINTNEXTLINE(readability-convert-member-functions-to-static)
    void construct_invalid_sequence() const
    {
      QFETCH(const QString, path);
      QVERIFY_EXCEPTION_THROWN(analyzer::sequence("Basketball", path),
                               analyzer::invalid_sequence);
    }

    // NOLINTNEXTLINE(readability-convert-member-functions-to-static)
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
  };
}  // namespace analyzer_test

QTEST_APPLESS_MAIN(analyzer_test::dataset_test)  // NOLINT
#include "dataset_test.moc"