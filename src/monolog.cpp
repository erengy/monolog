#include "monolog.hpp"

#include <array>
#include <chrono>
#include <cstdio>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>

#ifdef _WIN32
#include <windows.h>
#endif

namespace monolog {

namespace util {

std::string get_datetime(const std::string_view format) {
  using system_clock = std::chrono::system_clock;
  const auto now = system_clock::to_time_t(system_clock::now());

  std::tm tm = {};
  localtime_s(&tm, &now);

  std::stringstream ss;
  ss << std::put_time(&tm, format.data());

  return ss.str();
};

std::string get_filename(const std::string_view path) {
  return std::string{path.substr(path.find_last_of("/\\") + 1)};
};

std::wstring to_wstring(const std::string_view str) {
#ifdef _WIN32
  const auto multi_byte_to_wide_char = [&str](LPWSTR output, int size) {
    return ::MultiByteToWideChar(CP_UTF8, 0, str.data(),
                                 static_cast<int>(str.size()), output, size);
  };

  if (!str.empty()) {
    const int size = multi_byte_to_wide_char(nullptr, 0);
    if (size > 0) {
      std::wstring output(size, '\0');
      if (multi_byte_to_wide_char(&output.front(), size))
        return output;
    }
  }
#endif

  return {};
}

}  // namespace util

////////////////////////////////////////////////////////////////////////////////

Record::Record(const std::string_view text)
    : text_{text} {
}

const std::string& Record::to_string() const {
  return text_;
}

////////////////////////////////////////////////////////////////////////////////

void Log::write(const Level level, const Record& record, const Source& source) {
  std::lock_guard lock{mutex_};

  if (level >= level_) {
    const auto output = format(level, record, source);
    if (console_output_)
      write_to_console(output);
    if (debugger_output_)
      write_to_debugger(output);
    if (file_output_)
      write_to_file(output);
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

void Log::set_newline(const std::string_view newline) {
  newline_ = newline;
}

void Log::set_path(const std::string_view path) {
  path_ = path;
}

////////////////////////////////////////////////////////////////////////////////

std::string Log::format(const Level level, const Record& record,
                        const Source& source) const {
  const auto& text = record.to_string();

  const bool multiline = text.find_first_of("\r\n") != text.npos;
  const auto format = !multiline ?
      "%s [%s] %s:%d %s | %s" + newline_ :
      "%s [%s] %s:%d %s | >>" + newline_ + "%s" + newline_;

  const auto datetime = util::get_datetime("%Y-%m-%d %H:%M:%S");
  const auto filename = util::get_filename(source.file);

  const auto snprintf = [&](char* buffer, size_t buf_size) {
    return std::snprintf(buffer, buf_size, format.c_str(),
                         datetime.c_str(), to_level_string(level),
                         filename.c_str(), source.line, source.function.c_str(),
                         text.c_str());
  };

  const auto size = snprintf(nullptr, 0);
  std::string output(size + 1, '\0');
  snprintf(&output[0], output.size());
  output.pop_back();

  return output;
}

const char* Log::to_level_string(const Level level) const {
  static const std::array<const char*, 8> levels = {
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

void Log::write_to_console(const std::string& text) const {
  std::cout << text;
}

void Log::write_to_debugger(const std::string& text) const {
#if defined(_DEBUG) && defined(_WIN32)
  ::OutputDebugStringA(text.c_str());
#endif
}

void Log::write_to_file(const std::string& text) const {
  if (path_.empty())
    return;

#ifdef _WIN32
  HANDLE file_handle = ::CreateFileW(util::to_wstring(path_).c_str(),
      FILE_APPEND_DATA, FILE_SHARE_READ, nullptr, OPEN_ALWAYS,
      FILE_ATTRIBUTE_NORMAL, nullptr);

  if (file_handle != INVALID_HANDLE_VALUE) {
    const auto file_pointer = ::SetFilePointer(file_handle, 0, nullptr,
                                               FILE_END);
    if (file_pointer != INVALID_SET_FILE_POINTER) {
      DWORD bytes_written = 0;
      ::WriteFile(file_handle, text.data(), static_cast<DWORD>(text.size()),
                  &bytes_written, nullptr);
    }

    ::CloseHandle(file_handle);
  }
#else
  std::ofstream file{path_, std::ios::out | std::ios::binary | std::ios::app};

  if (file) {
    file.write(text.c_str(), text.size());
  }
#endif
}

}  // namespace monolog
