// Host unit tests for the debounce module.
//
// Build and run on the host (no Arduino required):
//   g++ -std=c++11 -I relay_to_rs485 -Wall -Wextra -Werror test/test_debounce.cpp -o /tmp/test_debounce && /tmp/test_debounce

#include <cassert>
#include <cstdint>
#include <cstdio>

#include "debounce.h"

using relay::Debounce;

namespace {

int failures = 0;

void check(const char* name, bool condition) {
  if (!condition) {
    std::printf("FAIL: %s\n", name);
    ++failures;
  }
}

}  // namespace

int main() {
  // Stable input passes through immediately after the debounce window.
  {
    Debounce d(30);
    check("stable true accepted after window", d.update(true, 0) == false);
    check("still false before window", d.update(true, 20) == false);
    check("true accepted at window", d.update(true, 30) == true);
    check("true stable", d.update(true, 100) == true);
  }

  // Bounce (glitch) resets the window and never settles.
  {
    Debounce d(30);
    d.update(true, 0);
    d.update(false, 10);  // bounce: resets stable timer
    d.update(true, 20);   // bounce again
    check("glitch keeps false", d.update(false, 40) == false);
    check("settles after window", d.update(false, 45) == false);
  }

  // Change after settling is picked up.
  {
    Debounce d(30);
    d.update(false, 0);
    d.update(false, 100);
    check("settled false", d.value() == false);
    d.update(true, 101);  // raw change resets timer
    check("no immediate flip", d.update(true, 102) == false);
    check("flips after window", d.update(true, 150) == true);
  }

  // Edge at exactly the window boundary.
  {
    Debounce d(30);
    d.update(true, 0);
    check("window edge inclusive", d.update(true, 30) == true);
  }

  // Large clock wrap (millis() rollover) is handled by unsigned arithmetic.
  {
    Debounce d(30);
    d.update(true, 0xFFFFFFF0u);
    check("wrap not immediate", d.update(true, 10) == false);
    check("wrap accepted", d.update(true, 30) == true);
  }

  // Boot priming: reset() trusts the initial reading immediately, so an
  // input already active at power-on is not reported as a spurious change.
  {
    Debounce d(30);
    d.reset(true, 0);
    check("reset trusts active input", d.value() == true);
    check("no spurious change at next poll", d.update(true, 50) == true);
  }

  // reset() to an inactive input behaves the same.
  {
    Debounce d(30);
    d.reset(false, 0);
    check("reset trusts inactive input", d.value() == false);
    check("stable inactive", d.update(false, 50) == false);
  }

  if (failures == 0) {
    std::printf("All tests passed.\n");
    return 0;
  }
  std::printf("%d test(s) failed.\n", failures);
  return 1;
}
