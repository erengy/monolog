#pragma once

#include <mutex>
#include <string>
#include <string_view>

namespace monolog {

enum class Level {
  Debug,
  Informational,
  Notice,
  Warning,
  Error,
  Critical,
  Alert,
  Emergency,
};

struct Source {
  std::string file;
  std::string function;
  int line = 0;
};

class Record {
public:
  explicit Record(const std::string_view text);

  const std::string& to_string() const;

private:
  std::string text_;
};

class Log {
public:
  void write(const Level level, const Record& record, const Source& source);

  void enable_console_output(const bool enabled);
  void enable_debugger_output(const bool enabled);
  void enable_file_output(const bool enabled);

  void set_level(const Level level);
  void set_newline(const std::string_view newline);
  void set_path(const std::string_view path);

private:
  std::string format(const Level level, const Record& record, const Source& source) const;
  const char* to_level_string(const Level level) const;

  void write_to_console(const std::string& text) const;
  void write_to_debugger(const std::string& text) const;
  void write_to_file(const std::string& text) const;

  Level level_ = Level::Debug;
  std::mutex mutex_;
  std::string newline_ = "\n";
  std::string path_;

  bool console_output_ = true;
  bool debugger_output_ = true;
  bool file_output_ = true;
};

inline Log log;

}  // namespace monolog

#define MONOLOG(level, text) \
    monolog::log.write(level, monolog::Record{text}, monolog::Source{__FILE__, __FUNCTION__, __LINE__})

#define MONOLOG_DEBUG(text)   MONOLOG(monolog::Level::Debug, text)
#define MONOLOG_INFO(text)    MONOLOG(monolog::Level::Informational, text)
#define MONOLOG_WARNING(text) MONOLOG(monolog::Level::Warning, text)
#define MONOLOG_ERROR(text)   MONOLOG(monolog::Level::Error, text)
