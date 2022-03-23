#ifndef ANALYZER_EXCEPTIONS_H
#define ANALYZER_EXCEPTIONS_H

#include <stdexcept>

namespace analyzer
{
  class invalid_sequence final: public std::runtime_error
  {
  public:
    invalid_sequence(const std::string& sequence_name,
                     const std::string& sequence_path,
                     const std::string& what):
      std::runtime_error {what}, m_name {sequence_name}, m_path {sequence_path}
    {
    }
    [[nodiscard]] inline auto name() const -> std::string { return m_name; }
    [[nodiscard]] inline auto path() const -> std::string { return m_path; }

  private:
    std::string m_name;
    std::string m_path;
  };

  class invalid_tracker final: public std::runtime_error
  {
  public:
    invalid_tracker(const std::string& tracker_name, const std::string& what):
      std::runtime_error {what}, m_name {tracker_name}
    {
    }
    [[nodiscard]] inline auto name() const -> std::string { return m_name; }

  private:
    std::string m_name;
  };
}  // namespace analyzer

#endif
