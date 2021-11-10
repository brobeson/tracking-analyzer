#include "tracking-analyzer/filesystem.h"
#include <QTest>

namespace analyzer_test
{
  class filesystem_test final: public QObject
  {
    // NOLINTNEXTLINE(modernize-use-trailing-return-type)
    Q_OBJECT

  private slots:
    void make_absolute_path_data() const
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
      QTest::newRow("Windows path") << R"(C:\Users\user\Videos\)"
                                    << R"(C:\Users\user\Videos\)";
    }

    void make_absolute_path() const
    {
      QFETCH(const QString, path);
      QTEST(analyzer::make_absolute_path(path), "absolute_path");
    }

    void basename_data() const
    {
      QTest::addColumn<QString>("path");
      QTest::addColumn<QString>("expected_basename");
      // clang-format off
      // Clang-format is splitting the rows onto multiple lines when it doesn't
      // need to.
      QTest::newRow("empty path") << "" << "";
      QTest::newRow("just /") << "/" << "";
      QTest::newRow("one directory") << "/usr" << "usr";
      QTest::newRow("one with trailing slash") << "/usr/" << "usr";
      QTest::newRow("two directories") << "/usr/lib" << "lib";
      QTest::newRow("two with trailing slash") << "/usr/lib/" << "lib";
      QTest::newRow("a relative file") << "main.h" << "main.h";
      QTest::newRow("an absolute file") << "/main.h" << "main.h";
      QTest::newRow("a file with parents")
        << "/usr/lib/libSomething.so" << "libSomething.so";
      // clang-format on
    }

    void basename() const
    {
      QFETCH(const QString, path);
      QTEST(analyzer::basename(path), "expected_basename");
    }
  };
}  // namespace analyzer_test

// NOLINTNEXTLINE(modernize-use-trailing-return-type)
QTEST_APPLESS_MAIN(analyzer_test::filesystem_test)
#include "filesystem_test.moc"
