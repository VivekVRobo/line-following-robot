#pragma once

#include <stdint.h>

struct RecoveryCommand {
  int left;
  int right;
  uint8_t phase;

  RecoveryCommand() : left(0), right(0), phase(0) {}
  RecoveryCommand(int leftValue, int rightValue, uint8_t phaseValue)
      : left(leftValue), right(rightValue), phase(phaseValue) {}
};

struct RecoveryConfig { int spinSpeed; int sweepSpeed; uint32_t initialSpinMs; uint32_t sweepPeriodMs; };

class RecoveryPlanner {
 public:
  explicit RecoveryPlanner(RecoveryConfig cfg) : cfg_(cfg) {}
  void begin(int lastDirection, uint32_t nowMs) {
    active_ = true;
    preferredDirection_ = lastDirection >= 0 ? 1 : -1;
    startedMs_ = nowMs;
  }
  void reset() { active_ = false; }
  bool active() const { return active_; }
  RecoveryCommand command(uint32_t nowMs) const {
    if (!active_) return RecoveryCommand();
    const uint32_t elapsed = nowMs - startedMs_;
    if (elapsed < cfg_.initialSpinMs) return spin(preferredDirection_, cfg_.spinSpeed, 1);
    const uint32_t sweepElapsed = elapsed - cfg_.initialSpinMs;
    const uint32_t segment = cfg_.sweepPeriodMs == 0 ? 0 : sweepElapsed / cfg_.sweepPeriodMs;
    const int direction = (segment % 2 == 0) ? preferredDirection_ : -preferredDirection_;
    return spin(direction, cfg_.sweepSpeed, static_cast<uint8_t>(2 + (segment % 2)));
  }
 private:
  static RecoveryCommand spin(int direction, int speed, uint8_t phase) {
    return direction >= 0 ? RecoveryCommand(speed, -speed, phase)
                          : RecoveryCommand(-speed, speed, phase);
  }
  RecoveryConfig cfg_;
  bool active_ = false;
  int preferredDirection_ = 1;
  uint32_t startedMs_ = 0;
};
