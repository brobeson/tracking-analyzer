#include "tracking-analyzer/exceptions.h"
#include <QTest>

using namespace std::literals::string_literals;

namespace analyzer_test
{
  class exceptions_test final: public QObject
  {
    // NOLINTNEXTLINE(modernize-use-trailing-return-type)
    Q_OBJECT

  private slots:
    void invalid_sequence_test() const
    {
      analyzer::invalid_sequence exc {
        "Deer", "~/Videos/otb/Deer", "what message?"};
      QCOMPARE(exc.name(), "Deer"s);
      QCOMPARE(exc.path(), "~/Videos/otb/Deer"s);
      QCOMPARE(exc.what(), "what message?");
    }

    void invalid_tracker_test() const
    {
      analyzer::invalid_tracker exc {"MDNet", "what message?"};
      QCOMPARE(exc.name(), "MDNet"s);
      QCOMPARE(exc.what(), "what message?");
    }
  };
}  // namespace analyzer_test

// NOLINTNEXTLINE(modernize-use-trailing-return-type)
QTEST_APPLESS_MAIN(analyzer_test::exceptions_test)
#include "exceptions_test.moc"
