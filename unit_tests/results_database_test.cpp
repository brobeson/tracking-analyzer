#include "tracking-analyzer/tracking_results.h"
#include <QTest>

using namespace std::literals::string_literals;

namespace analyzer_test
{
  namespace
  {
    // Exceptions during construction of these objects are unlikely. If
    // exceptions are thrown, well, this is a unit test. I'll deal with it.
    // NOLINTNEXTLINE(cert-err58-cpp)
    const std::vector<std::string> expected_names {"MDNet", "VITAL"};

    auto make_database()
    {
      analyzer::results_database db;
      db.trackers() = {analyzer::tracker_results {
                         "MDNet", analyzer::tracker_results::sequence_list {}},
                       analyzer::tracker_results {
                         "VITAL", analyzer::tracker_results::sequence_list {}}};
      return db;
    }
  }  // namespace

  class results_database_test final: public QObject
  {
    // NOLINTNEXTLINE(modernize-use-trailing-return-type)
    Q_OBJECT

  private slots:
    void size_test() const
    {
      const auto db {make_database()};
      QCOMPARE(analyzer::size(db), 2ul);
    }

    void read_only_trackers_access_test() const
    {
      const auto db {make_database()};
      QCOMPARE(db.trackers().size(), 2ul);
    }

    void read_write_trackers_access_test() const
    {
      auto db {make_database()};
      QCOMPARE(db.trackers().size(), 2ul);
      db.trackers().emplace_back("TMFT",
                                 analyzer::tracker_results::sequence_list {});
      QCOMPARE(db.trackers().size(), 3ul);
    }

    void read_only_tracker_lookup_test() const
    {
      const auto db {make_database()};
      QCOMPARE(db["MDNet"].name(), "MDNet"s);
    }

    void read_only_tracker_lookup_throw_test() const
    {
      const auto db {make_database()};
      QVERIFY_EXCEPTION_THROWN([[maybe_unused]] const auto t {db["mdnet"]},
                               analyzer::invalid_tracker);
    }

    void read_write_tracker_lookup_test() const
    {
      auto db {make_database()};
      db["MDNet"].sequences().clear();
      QVERIFY(db["MDNet"].sequences().empty());
    }

    void read_write_tracker_lookup_throw_test() const
    {
      auto db {make_database()};
      QVERIFY_EXCEPTION_THROWN(db["mdnet"].sequences().clear(),
                               analyzer::invalid_tracker);
    }

    void iteration_test() const
    {
      const auto db {make_database()};
      analyzer::results_database::size_type i {0ul};
      for (const auto& tracker : db)
      {
        QCOMPARE(tracker.name(), expected_names.at(i));
        ++i;
      }
    }
  };
}  // namespace analyzer_test

// NOLINTNEXTLINE(modernize-use-trailing-return-type)
QTEST_APPLESS_MAIN(analyzer_test::results_database_test)
#include "results_database_test.moc"
