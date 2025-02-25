#pragma once
#include "types.h"

enum ControllerButton : uint8_t {
  RIGHT = 1 << 0,
  LEFT = 1 << 1,
  DOWN = 1 << 2,
  UP = 1 << 3,
  START = 1 << 4,
  SELECT = 1 << 5,
  B = 1 << 6,
  A = 1 << 7,
};

class ControllerState {
 protected:
  uint8_t state_ = 0;

 public:
  uint8_t state() const { return state_; }
  void push_button(ControllerButton mask) { state_ |= mask; }
  void unpush_button(ControllerButton mask) { state_ &= ~mask; }
  void toggle_button(ControllerButton mask) { state_ ^= mask; }
};
