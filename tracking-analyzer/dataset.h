#ifndef TRACKING_ANALYZER_DATASET_H
#define TRACKING_ANALYZER_DATASET_H

#include <QStringList>
#include <QVector>
#include <stdexcept>

namespace analyzer
{
  class invalid_sequence final: public std::runtime_error
  {
  public:
    invalid_sequence(const QString& name,
                     const QString& path,
                     const std::string& what);
    QString name() const;
    QString path() const;

  private:
    QString m_name;
    QString m_path;
  };

  class sequence final
  {
  public:
    sequence() = default;
    sequence(const QString& name, const QString& path);

    QString name() const;
    QStringList frame_paths() const;

  private:
    QString m_name;
    QString m_root_path;
    QStringList m_frame_paths;
  };

  class dataset final
  {
  public:
    dataset() = default;
    dataset(const QString& root_directory,
            const QVector<analyzer::sequence>& sequences);

    // [[nodiscard]] const QStringList& sequence_names() const noexcept;

    [[nodiscard]] const QString& root_path() const noexcept;

    [[nodiscard]] const QVector<analyzer::sequence>& sequences() const noexcept;

  private:
    QString m_root_directory;
    // QStringList m_sequence_names;
    QVector<analyzer::sequence> m_sequences;
  };

  QString make_absolute_path(const QString& path);

  dataset load_dataset(const QString& path);

  QStringList sequence_names(const QVector<analyzer::sequence>& sequences);
}  // namespace analyzer

#endif