#pragma once

#include <ctype.h>

enum class CommandType { None, Start, Stop, TelemetryOn, TelemetryOff, Status, Help, Unknown };

inline bool equalsIgnoreCase(const char* a, const char* b) {
  while (*a && *b) {
    if (tolower(static_cast<unsigned char>(*a)) !=
        tolower(static_cast<unsigned char>(*b))) return false;
    ++a;
    ++b;
  }
  return *a == '\0' && *b == '\0';
}

inline CommandType parseCommand(const char* line) {
  if (!line) return CommandType::None;

  while (*line && isspace(static_cast<unsigned char>(*line))) ++line;
  if (*line == '\0') return CommandType::None;

  const char* end = line;
  while (*end) ++end;
  while (end > line && isspace(static_cast<unsigned char>(end[-1]))) --end;

  const size_t length = static_cast<size_t>(end - line);
  if (length == 0) return CommandType::None;

  char normalized[32] = {};
  if (length >= sizeof(normalized)) return CommandType::Unknown;
  for (size_t i = 0; i < length; ++i) normalized[i] = line[i];

  if (equalsIgnoreCase(normalized, "START")) return CommandType::Start;
  if (equalsIgnoreCase(normalized, "STOP")) return CommandType::Stop;
  if (equalsIgnoreCase(normalized, "TELEM ON")) return CommandType::TelemetryOn;
  if (equalsIgnoreCase(normalized, "TELEM OFF")) return CommandType::TelemetryOff;
  if (equalsIgnoreCase(normalized, "STATUS")) return CommandType::Status;
  if (equalsIgnoreCase(normalized, "HELP")) return CommandType::Help;
  return CommandType::Unknown;
}
