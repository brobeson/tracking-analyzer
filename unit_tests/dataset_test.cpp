#include "tracking-analyzer/dataset.h"
#include <QDebug>
#include <QTest>
#include <iostream>

Q_DECLARE_METATYPE(std::string)                // NOLINT
Q_DECLARE_METATYPE(analyzer::sequence_record)  // NOLINT
Q_DECLARE_METATYPE(analyzer::dataset)          // NOLINT

using namespace std::literals::string_literals;

namespace analyzer
{
  // This operator must not be in an unnamed namespace, or the compiler can't
  // find it for later operations.
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-declarations"
  [[nodiscard]] auto toString(const std::string& s)
  {
    return QTest::toString(QString::fromStdString(s));
  }

  [[nodiscard]] auto toString(const dataset& db)
  {
    return QTest::toString(QString::fromStdString(db.root_path()));
  }

  [[nodiscard]] auto operator==(const analyzer::frame_record& a,
                                const analyzer::frame_record& b)
  {
    return a.image_path() == b.image_path();
  }

  [[nodiscard]] auto operator==(const analyzer::sequence_record& a,
                                const analyzer::sequence_record& b)
  {
    return a.name() == b.name() && a.frames() == b.frames();
  }

  [[nodiscard]] auto operator==(const analyzer::dataset& a,
                                const analyzer::dataset& b)
  {
    return a.root_path() == b.root_path() && a.sequences() == b.sequences();
  }
#pragma GCC diagnostic pop
}  // namespace analyzer

namespace analyzer_test
{
  namespace
  {
    auto make_dataset()
    {
      return analyzer::dataset {"a path",
                                {{"Deer", "Deer path", {}, {}},
                                 {"Basketball", "Basketball path", {}, {}}}};
    }
  }  // namespace

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

    void construct_dataset_test() const
    {
      const auto db {make_dataset()};
      QCOMPARE(db.root_path(), "a path"s);
      QCOMPARE(db.sequences().size(), 2ul);
    }

    void modify_sequences_test() const
    {
      auto db {make_dataset()};
      QCOMPARE(db.sequences().size(), 2ul);
      db.sequences().emplace_back("David",
                                  "David path",
                                  analyzer::tag_list {},
                                  analyzer::sequence_record::frame_list {});
      QCOMPARE(db.sequences().size(), 3ul);
    }

    void sequence_lookup_throw_test() const
    {
      const auto db {make_dataset()};
      QVERIFY_EXCEPTION_THROWN([[maybe_unused]] auto s {db["deer"]},
                               analyzer::invalid_sequence);
    }

    void sequence_lookup_test() const
    {
      auto db {make_dataset()};
      QCOMPARE(db["Deer"].root_path(), "Deer path"s);
      QVERIFY(db["Deer"].frames().empty());
      db["Deer"].frames().emplace_back("frame path");
      QCOMPARE(analyzer::size(db["Deer"]), 1ul);
    }

    void challenge_tags_test() const
    {
      const auto tags {make_dataset().challenge_tags()};
      QCOMPARE(tags,
               analyzer::tag_list({"illumination variation",
                                   "scale variation",
                                   "occlusion",
                                   "deformation",
                                   "motion blur",
                                   "fast motion",
                                   "in-plane rotation",
                                   "out-of-plane rotation",
                                   "out-of-view",
                                   "background clutters",
                                   "low resolution"}));
    }

    // void load_dataset_data() const
    // {
    //   QTest::addColumn<std::string>("path");
    //   QTest::addColumn<analyzer::dataset_db>("dataset");
    //   // QTest::newRow("nonexistent directory")
    //   //   << "ghost/"
    //   //   << analyzer::dataset {"ghost/", QVector<analyzer::sequence> {}};
    //   // QTest::newRow("empty directory")
    //   //   << "empty_dataset/"
    //   //   << analyzer::dataset {"empty_dataset/",
    //   QVector<analyzer::sequence>
    //   //   {}};
    //   QTest::newRow("partial OTB")
    //     << "test_dataset"s
    //     << analyzer::dataset_db {
    //          "test_dataset",
    //          analyzer::dataset_db::sequence_list {
    //            analyzer::sequence_record {
    //              "Basketball", "test_dataset/Basketball", {}, {}},
    //            analyzer::sequence_record {
    //              "Biker",
    //              "test_dataset/Biker",
    //              {},
    //              analyzer::sequence_record::frame_list {
    //                analyzer::frame_record
    //                {"test_dataset/Biker/img/0001.jpg"},
    //                analyzer::frame_record
    //                {"test_dataset/Biker/img/0002.jpg"},
    //                analyzer::frame_record
    //                {"test_dataset/Biker/img/0003.jpg"}}},
    //            analyzer::sequence_record {
    //              "Bird1", "test_dataset/Bird1", {}, {}},
    //            analyzer::sequence_record {
    //              "Bird2", "test_dataset/Bird2", {}, {}},
    //          }};
    // }

    // void load_dataset() const
    // {
    //   QFETCH(const std::string, path);
    //   // const auto ds {analyzer::load_dataset(path)};
    //   // QFETCH(const analyzer::dataset, dataset);
    //   // QCOMPARE(ds.sequences().size(), dataset.sequences().size());
    //   // for (int i {0}; i < ds.sequences().size(); ++i)
    //   // {
    //   //   QCOMPARE(ds.sequences()[i].name(), dataset.sequences()[i].name());
    //   //   QCOMPARE(ds.sequences()[i].frames().size(),
    //   //            dataset.sequences()[i].frames().size());
    //   // }
    //   // QCOMPARE(ds.root_path(), QString("test_dataset"));
    //   QTEST(analyzer::load_dataset_from_disk(path), "dataset");
    // }

    void sequence_names_data() const
    {
      QTest::addColumn<analyzer::dataset>("db");
      QTest::addColumn<std::vector<std::string>>("expected_names");
      QTest::newRow("empty list")
        << analyzer::dataset {} << std::vector<std::string> {};
      QTest::newRow("one sequence")
        << analyzer::dataset {"",
                              analyzer::dataset::sequence_list {
                                {"Biker", "test_dataset/Biker", {}, {}}}}
        << std::vector<std::string> {"Biker"};
      QTest::newRow("two sequences")
        << analyzer::dataset {"",
                              analyzer::dataset::sequence_list {
                                {"Biker", "test_dataset/Biker", {}, {}},
                                {"Dancer", "test_dataset/Biker", {}, {}}}}
        << std::vector<std::string> {"Biker", "Dancer"};
    }

    void sequence_names() const
    {
      QFETCH(const analyzer::dataset, db);
      QTEST(analyzer::sequence_names(db), "expected_names");
    }
  };
}  // namespace analyzer_test

// NOLINTNEXTLINE(modernize-use-trailing-return-type)
QTEST_APPLESS_MAIN(analyzer_test::dataset_test)
#include "dataset_test.moc"