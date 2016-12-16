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
