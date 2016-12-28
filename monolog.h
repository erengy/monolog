/*
MIT License

Copyright (c) 2016 Eren Okka

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#pragma once

#include <mutex>
#include <string>

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
  int line;
};

class Record {
public:
  Record(const std::string& text);
  Record(const std::wstring& text);
  Record(const char* text);
  Record(const wchar_t* text);

  operator std::string() const;

private:
  std::string text_;
};

class Log {
public:
  void Write(const Level level, const Record& record, const Source& source);

  void set_level(const Level level);
  void set_path(const std::string& path);

private:
  std::string Format(const Level level, const Source& source, std::string text) const;
  const char* LevelString(const Level level) const;

  void WriteToConsole(const std::string& text) const;
  void WriteToDebugger(const std::string& text) const;
  void WriteToFile(const std::string& text) const;

  Level level_ = Level::Debug;
  std::mutex mutex_;
  std::string path_;
};

extern Log log;

}  // namespace monolog

#define MONOLOG(level, text) \
    monolog::log.Write(level, text, {__FILE__, __FUNCTION__, __LINE__})

#define LOG_DEBUG(text)   MONOLOG(monolog::Level::Debug, text)
#define LOG_INFO(text)    MONOLOG(monolog::Level::Informational, text)
#define LOG_WARNING(text) MONOLOG(monolog::Level::Warning, text)
#define LOG_ERROR(text)   MONOLOG(monolog::Level::Error, text)

#define LOGD(text) LOG_DEBUG(text)
#define LOGI(text) LOG_INFO(text)
#define LOGW(text) LOG_WARNING(text)
#define LOGE(text) LOG_ERROR(text)
