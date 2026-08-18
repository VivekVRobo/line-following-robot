#pragma once

#include <cctype>

enum class CommandType { None, Start, Stop, TelemetryOn, TelemetryOff, Status, Help, Unknown };

inline bool equalsIgnoreCase(const char* a, const char* b) {
  while (*a && *b) {
    if (std::tolower(static_cast<unsigned char>(*a)) !=
        std::tolower(static_cast<unsigned char>(*b))) return false;
    ++a; ++b;
  }
  return *a == '\0' && *b == '\0';
}

inline CommandType parseCommand(const char* line) {
  if (!line || line[0] == '\0') return CommandType::None;
  if (equalsIgnoreCase(line, "START")) return CommandType::Start;
  if (equalsIgnoreCase(line, "STOP")) return CommandType::Stop;
  if (equalsIgnoreCase(line, "TELEM ON")) return CommandType::TelemetryOn;
  if (equalsIgnoreCase(line, "TELEM OFF")) return CommandType::TelemetryOff;
  if (equalsIgnoreCase(line, "STATUS")) return CommandType::Status;
  if (equalsIgnoreCase(line, "HELP")) return CommandType::Help;
  return CommandType::Unknown;
}
