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

#include <array>
#include <chrono>
#include <codecvt>
#include <cstdio>
#include <ctime>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>

#ifdef _WIN32
#include <windows.h>
#endif

#include "monolog.h"

namespace monolog {

Log log;

////////////////////////////////////////////////////////////////////////////////

namespace util {

std::string get_datetime(const char* format) {
  using system_clock = std::chrono::system_clock;
  const auto now = system_clock::to_time_t(system_clock::now());

  std::tm tm = {};
  localtime_s(&tm, &now);

  std::stringstream ss;
  ss << std::put_time(&tm, format);

  return ss.str();
};

std::string get_filename(const std::string& path) {
  return path.substr(path.find_last_of("/\\") + 1);
};

std::string to_string(const std::wstring& str) {
#ifdef _WIN32
  if (!str.empty()) {
    auto length = ::WideCharToMultiByte(CP_UTF8, 0, str.c_str(), -1,
                                        nullptr, 0, nullptr, nullptr);
    if (length > 0) {
      std::string output(length, '\0');
      ::WideCharToMultiByte(CP_UTF8, 0, str.c_str(), -1,
                            &output[0], length, nullptr, nullptr);
      return output;
    }
  }
#else
  std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
  try {
    return converter.to_bytes(str);
  } catch (const std::range_error&) {
    return std::string(str.begin(), str.end());
  }
#endif
  return std::string();
}

void trim_right(std::string& str, const std::string& chars) {
  const auto pos = str.find_last_not_of(chars);
  if (pos != std::string::npos && pos + 1 < str.size()) {
    str.resize(pos + 1);
  }
}

}  // namespace util

////////////////////////////////////////////////////////////////////////////////

Record::Record(const std::string& text)
    : text_(text) {
}

Record::Record(const std::wstring& text)
    : text_(util::to_string(text)) {
}

Record::Record(const char* text)
    : Record(std::string(text)) {
}

Record::Record(const wchar_t* text)
    : Record(std::wstring(text)) {
}

Record::operator std::string() const {
  return text_;
}

////////////////////////////////////////////////////////////////////////////////

void Log::Write(const Level level, const Record& record, const Source& source) {
  std::lock_guard<std::mutex> lock(mutex_);

  if (level >= level_) {
    const auto output = Format(level, source, record);
    if (console_output_)
      WriteToConsole(output);
    if (debugger_output_)
      WriteToDebugger(output);
    if (file_output_)
      WriteToFile(output);
  }
}

void Log::enable_console_output(const bool enabled) {
  console_output_ = enabled;
}

void Log::enable_debugger_output(const bool enabled) {
  debugger_output_ = enabled;
}

void Log::enable_file_output(const bool enabled) {
  file_output_ = enabled;
}

void Log::set_level(const Level level) {
  level_ = level;
}

void Log::set_path(const std::string& path) {
  path_ = path;
}

////////////////////////////////////////////////////////////////////////////////

std::string Log::Format(const Level level, const Source& source,
                        std::string text) const {
  util::trim_right(text, "\r\n");
  const bool multiline = text.find_first_of("\r\n") != std::string::npos;

  const auto datetime = util::get_datetime("%Y-%m-%d %H:%M:%S");
  const auto filename = util::get_filename(source.file);
  const auto format = !multiline ? "%s [%s] %s:%d %s | %s\n" :
                                   "%s [%s] %s:%d %s | >>\n%s\n";

  auto snprintf = [&](char* buffer, size_t buf_size) {
    return std::snprintf(buffer, buf_size, format,
                         datetime.c_str(), LevelString(level),
                         filename.c_str(), source.line, source.function.c_str(),
                         text.c_str());
  };

  const auto size = snprintf(nullptr, 0);
  std::string output(size + 1, '\0');
  snprintf(&output[0], output.size());
  output.pop_back();

  return output;
}

const char* Log::LevelString(const Level level) const {
  static const std::array<char*, 8> levels = {
    "Debug",
    "Informational",
    "Notice",
    "Warning",
    "Error",
    "Critical",
    "Alert",
    "Emergency",
  };

  return levels[static_cast<size_t>(level)];
}

void Log::WriteToConsole(const std::string& text) const {
  std::cout << text;
}

void Log::WriteToDebugger(const std::string& text) const {
#if defined(_DEBUG) && defined(_WIN32)
  ::OutputDebugStringA(text.c_str());
#endif
}

void Log::WriteToFile(const std::string& text) const {
  if (path_.empty()) {
    return;
  }

  std::ofstream stream;
  stream.open(path_, std::ofstream::app | std::ios::binary | std::ofstream::out);

  if (stream.is_open()) {
    stream.write(text.c_str(), text.size());
    stream.close();
  }
}

}  // namespace monolog
