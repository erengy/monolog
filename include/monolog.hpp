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
  explicit Record(const std::wstring_view text);

  const std::string& to_string() const;

private:
  std::string text_;
};

class Log {
public:
  void Write(const Level level, const Record& record, const Source& source);

  void enable_console_output(const bool enabled);
  void enable_debugger_output(const bool enabled);
  void enable_file_output(const bool enabled);

  void set_level(const Level level);
  void set_newline(const std::string_view newline);
  void set_path(const std::string_view path);
  void set_path(const std::wstring_view path);

private:
  std::string Format(const Level level, const Record& record, const Source& source) const;
  const char* LevelString(const Level level) const;

  void WriteToConsole(const std::string& text) const;
  void WriteToDebugger(const std::string& text) const;
  void WriteToFile(const std::string& text) const;

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
    monolog::log.Write(level, monolog::Record{text}, monolog::Source{__FILE__, __FUNCTION__, __LINE__})

#define MONOLOG_DEBUG(text)   MONOLOG(monolog::Level::Debug, text)
#define MONOLOG_INFO(text)    MONOLOG(monolog::Level::Informational, text)
#define MONOLOG_WARNING(text) MONOLOG(monolog::Level::Warning, text)
#define MONOLOG_ERROR(text)   MONOLOG(monolog::Level::Error, text)
