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

Record::Record(const std::string& text)
    : text_(text) {
}

Record::Record(const std::wstring& text) {
  auto to_string = [](const std::wstring& str) {
    static std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
    try {
      return converter.to_bytes(str);
    } catch (const std::range_error&) {
      return std::string(str.begin(), str.end());
    }
  };

  text_ = to_string(text);
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
    const auto output = Format(record, source);
    WriteToConsole(output);
    WriteToDebugOutput(output);
    WriteToFile(output);
  }
}

void Log::set_level(const Level level) {
  level_ = level;
}

void Log::set_path(const std::string& path) {
  path_ = path;
}

////////////////////////////////////////////////////////////////////////////////

std::string Log::Format(std::string text, const Source& source) const {
  auto trim_right = [](std::string& str, const std::string& chars) {
    const auto pos = str.find_last_not_of(chars);
    if (pos != std::string::npos && pos + 1 < str.size()) {
      str.resize(pos + 1);
    }
  };

  auto get_datetime = []() {
    using system_clock = std::chrono::system_clock;
    const auto now = system_clock::to_time_t(system_clock::now());
    std::tm tm = {};
    gmtime_s(&tm, &now);
    std::stringstream ss;
    ss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");
    return ss.str();
  };

  auto get_filename = [](const std::string& path) {
    return path.substr(path.find_last_of("/\\") + 1);
  };

  trim_right(text, "\r\n");
  const bool multiline = text.find_first_of("\r\n") != std::string::npos;

  const auto datetime = get_datetime();
  const auto filename = get_filename(source.file);
  const auto format = !multiline ? "%s [%s] %s:%d %s | %s\n" :
                                   "%s [%s] %s:%d %s | >>\n%s\n";

  auto snprintf = [&](char* buffer, size_t buf_size) {
    return std::snprintf(buffer, buf_size, format,
                         datetime.c_str(), LevelString(level_),
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

void Log::WriteToDebugOutput(const std::string& text) const {
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
