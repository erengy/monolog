#pragma once

#include <string>

#include <windows/win/thread.h>

enum SeverityLevels {
  LevelEmergency,
  LevelAlert,
  LevelCritical,
  LevelError,
  LevelWarning,
  LevelNotice,
  LevelInformational,
  LevelDebug
};

class Logger {
public:
  Logger();
  virtual ~Logger() {}

  void Log(int severity_level, const std::wstring& file, int line,
           const std::wstring& function, std::wstring text, bool raw);
  void SetOutputPath(const std::wstring& path);
  void SetSeverityLevel(int severity_level);

private:
  win::CriticalSection critical_section_;
  std::wstring output_path_;
  int severity_level_;
};

extern class Logger Logger;

#ifndef LOG
#define LOG(level, text) \
  Logger.Log(level, __FILEW__, __LINE__, __FUNCTIONW__, text, false)
#endif
#ifndef LOGR
#define LOGR(level, text) \
  Logger.Log(level, __FILEW__, __LINE__, __FUNCTIONW__, text, true)
#endif
