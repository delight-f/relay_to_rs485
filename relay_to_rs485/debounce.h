/*
 * Debounce — host-testable input debouncer.
 *
 * Pure C++11, no Arduino or ESP-IDF dependencies, so it compiles and
 * unit-tests natively on the host. The firmware feeds it raw pin
 * readings and a millisecond clock; it reports a stable value once the
 * input has been unchanged for `debounce_ms`.
 *
 * SPDX-License-Identifier: CC-BY-SA-4.0
 */

#pragma once

#include <cstdint>

namespace relay {

class Debounce {
 public:
  /**
   * @param debounce_ms  minimum time a raw value must be stable (ms);
   *                     default 0 disables debouncing until reassigned
   */
  explicit Debounce(uint32_t debounce_ms = 0) : debounce_ms_(debounce_ms) {}

  /**
   * Feed one raw sample.
   *
   * @param raw    current raw reading (already active-level mapped)
   * @param now_ms monotonic millisecond clock
   * @return the debounced value once it has been stable for
   *         `debounce_ms`, otherwise the previous debounced value
   */
  bool update(bool raw, uint32_t now_ms) {
    if (raw != last_raw_) {
      last_raw_ = raw;
      stable_since_ms_ = now_ms;
      return debounced_;
    }
    if (now_ms - stable_since_ms_ >= debounce_ms_) {
      debounced_ = raw;
    }
    return debounced_;
  }

  /** Current debounced value without consuming a sample. */
  bool value() const { return debounced_; }

 private:
  uint32_t debounce_ms_;
  bool last_raw_ = false;
  bool debounced_ = false;
  uint32_t stable_since_ms_ = 0;
};

}  // namespace relay