#include "tracking-analyzer/dataset.h"
#include <QTest>

namespace analyzer_test
{
  using namespace std::literals::string_literals;

  class frame_record_test final: public QObject
  {
    // NOLINTNEXTLINE(modernize-use-trailing-return-type)
    Q_OBJECT

  private slots:
    void construct_default_frame_record() const
    {
      const analyzer::frame_record frame;
      QVERIFY(frame.image_path().empty());
      const auto box {frame.target_box()};
      QCOMPARE(box.x, 0.0f);
      QCOMPARE(box.y, 0.0f);
      QCOMPARE(box.width, 0.0f);
      QCOMPARE(box.height, 0.0f);
    }

    void construct_frame_record() const
    {
      const analyzer::frame_record frame {"a path", {1.0f, 1.0f, 1.0f, 1.0f}};
      QCOMPARE(frame.image_path(), "a path"s);
      const auto box {frame.target_box()};
      QCOMPARE(box.x, 1.0f);
      QCOMPARE(box.y, 1.0f);
      QCOMPARE(box.width, 1.0f);
      QCOMPARE(box.height, 1.0f);
    }
  };
}  // namespace analyzer_test

// NOLINTNEXTLINE(modernize-use-trailing-return-type)
QTEST_APPLESS_MAIN(analyzer_test::frame_record_test)
#include "frame_record_test.moc"
