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

#include <iostream>
#include <fstream>

#include "foreach.h"
#include "log.h"
#include "string.h"
#include "time.h"

const char* SeverityLevels[] = {
  "Emergency",
  "Alert",
  "Critical",
  "Error",
  "Warning",
  "Notice",
  "Informational",
  "Debug"
};

class Logger Logger;

Logger::Logger()
    : severity_level_(LevelDebug) {
}

void Logger::Log(int severity_level, const std::wstring& file, int line,
                 const std::wstring& function, std::wstring text, bool raw) {
  win::Lock lock(critical_section_);

  if (severity_level <= severity_level_) {
    std::string output_text;

    if (raw) {
      output_text += WstrToStr(text);

    } else {
      Trim(text, L" \r\n");

      output_text +=
          WstrToStr(std::wstring(GetDate()) + L" " + GetTime() + L" ") +
          "[" + std::string(SeverityLevels[severity_level]) + "] " +
          WstrToStr(GetFileName(file) + L":" + ToWstr(line) + L" " + function + L" | ");

      std::string padding(output_text.length(), ' ');
      std::vector<std::wstring> lines;
      Split(text, L"\n", lines);
      foreach_(it, lines) {
        Trim(*it, L" \r");
        if (!it->empty()) {
          if (it != lines.begin())
            output_text += padding;
          output_text += WstrToStr(*it + L"\r\n");
        }
      }
    }

#ifdef _DEBUG
    OutputDebugStringA(output_text.c_str());
#endif

    if (!output_path_.empty()) {
      std::ofstream stream;
      stream.open(output_path_, std::ofstream::app |
                                std::ios::binary |
                                std::ofstream::out);
      if (stream.is_open()) {
        stream.write(output_text.c_str(), output_text.size());
        stream.close();
      }
    }
  }
}

void Logger::SetOutputPath(const std::wstring& path) {
  output_path_ = path;
}

void Logger::SetSeverityLevel(int severity_level) {
  severity_level_ = severity_level;
}
