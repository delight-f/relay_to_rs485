/*
 * Relay-to-RS485 bridge firmware (ESP32-S2 / lolin_s2_mini).
 *
 * Reads up to `NUM_CHANNELS` active-low digital inputs (relay contacts)
 * and mirrors their debounced state onto a Modbus RTU RS-485 bus as
 * coils, so an external master can switch the corresponding output
 * relays. Inputs are filtered by both the ESP32 hardware glitch filter
 * and a software debounce.
 *
 * SPDX-License-Identifier: CC-BY-SA-4.0
 */

#include <Arduino.h>
#include <ModbusMaster.h>
#include <driver/gpio_filter.h>

#include "debounce.h"

// ---------------------------------------------------------------------------
// Configuration
// ---------------------------------------------------------------------------

// Modbus slave ID for the relay board.
constexpr uint8_t kModbusSlaveId = 255;

// RS-485 UART settings (the board's transceiver is wired to these pins).
constexpr unsigned long kModbusBaud = 9600;
constexpr uint8_t kModbusRxPin = 31;
constexpr uint8_t kModbusTxPin = 35;

// Debug serial baud.
constexpr unsigned long kDebugBaud = 115200;

// Number of input/coil pairs.
constexpr size_t kNumChannels = 5;

// Digital input pins wired to the input relays.
constexpr uint8_t kInputPins[kNumChannels] = {15, 17, 18, 19, 20};

// Modbus coil addresses corresponding to each input pin.
constexpr uint16_t kCoilAddresses[kNumChannels] = {0x0000, 0x0001, 0x0002,
                                                   0x0003, 0x0004};

// Poll interval — paces the loop so the RS-485 bus is not hammered and
// inputs get a stable read.
constexpr unsigned long kPollIntervalMs = 50;

// Software debounce: input must read the same value for this long before
// it is accepted as a real state change (hardware glitch filter is
// independent of and complements this).
constexpr unsigned long kDebounceMs = 30;

// Retry attempts for a failed Modbus write.
constexpr uint8_t kMaxWriteRetries = 3;

// ---------------------------------------------------------------------------
// State
// ---------------------------------------------------------------------------

ModbusMaster node;
relay::Debounce debouncers[kNumChannels];

// Last value actually written to Modbus for each channel.
bool lastSentState[kNumChannels] = {};

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

// Reads the pin and applies the active-low mapping: pull-up means HIGH =
// relay not active (false), LOW = relay active (true).
bool readMappedInput(uint8_t pin) { return digitalRead(pin) == LOW; }

// Writes a coil with retries. Returns true on success.
bool writeCoilWithRetry(uint16_t address, bool value, size_t channelIndex) {
  for (uint8_t attempt = 1; attempt <= kMaxWriteRetries; ++attempt) {
    const uint8_t result = node.writeSingleCoil(address, value);
    if (result == node.ku8MBSuccess) {
      return true;
    }
    Serial.printf("Channel %u: coil 0x%04X write failed (code 0x%02X), "
                  "attempt %u/%u\n",
                  channelIndex, address, result, attempt, kMaxWriteRetries);
    delay(10);  // brief pause before retrying, avoids re-flooding a busy bus
  }
  Serial.printf("Channel %u: coil 0x%04X write abandoned after %u attempts\n",
                channelIndex, address, kMaxWriteRetries);
  return false;
}

// ---------------------------------------------------------------------------
// Setup
// ---------------------------------------------------------------------------

void setup() {
  Serial.begin(kDebugBaud);

  Serial1.begin(kModbusBaud, SERIAL_8E1, kModbusRxPin, kModbusTxPin);
  node.begin(kModbusSlaveId, Serial1);

  for (size_t i = 0; i < kNumChannels; ++i) {
    pinMode(kInputPins[i], INPUT_PULLUP);
    debouncers[i] = relay::Debounce(kDebounceMs);
  }

  // Configure and enable a hardware glitch filter on each input pin.
  // Each pin needs its own config struct and its own handle — the handle
  // is populated by gpio_new_pin_glitch_filter, not declared empty and
  // enabled directly. Handles are intentionally leaked: they must stay
  // alive for the life of the program.
  for (size_t i = 0; i < kNumChannels; ++i) {
    const gpio_pin_glitch_filter_config_t filterConfig = {
        .clk_src = GLITCH_FILTER_CLK_SRC_DEFAULT,
        .gpio_num = static_cast<gpio_num_t>(kInputPins[i]),
    };

    gpio_glitch_filter_handle_t filterHandle = nullptr;
    esp_err_t err = gpio_new_pin_glitch_filter(&filterConfig, &filterHandle);
    if (err != ESP_OK) {
      Serial.printf("Pin %u: failed to create glitch filter (err %d)\n",
                    kInputPins[i], err);
      continue;
    }

    err = gpio_glitch_filter_enable(filterHandle);
    if (err != ESP_OK) {
      Serial.printf("Pin %u: failed to enable glitch filter (err %d)\n",
                    kInputPins[i], err);
    }
  }

  // Prime state so the first loop iteration only sends coils that genuinely
  // need setting, rather than assuming a stale default. reset() trusts the
  // boot reading immediately, so an input that is already active at power-on
  // is not reported as a spurious change.
  for (size_t i = 0; i < kNumChannels; ++i) {
    const bool initial = readMappedInput(kInputPins[i]);
    debouncers[i].reset(initial, millis());

    if (writeCoilWithRetry(kCoilAddresses[i], initial, i)) {
      lastSentState[i] = initial;
    } else {
      // Initial write failed: force a resend attempt on the first loop pass
      // by marking the sent state as unknown/opposite.
      lastSentState[i] = !initial;
    }
  }
}

// ---------------------------------------------------------------------------
// Loop
// ---------------------------------------------------------------------------

void loop() {
  static unsigned long lastPollTime = 0;
  const unsigned long now = millis();

  if (now - lastPollTime < kPollIntervalMs) {
    return;  // not time to poll yet — keeps bus traffic paced
  }
  lastPollTime = now;

  for (size_t i = 0; i < kNumChannels; ++i) {
    const bool raw = readMappedInput(kInputPins[i]);
    const bool stable = debouncers[i].update(raw, now);

    // Debounced value differs from what was last sent — send it.
    if (stable != lastSentState[i]) {
      if (writeCoilWithRetry(kCoilAddresses[i], stable, i)) {
        lastSentState[i] = stable;
      }
      // on failure, lastSentState is left unchanged so it retries next poll
    }
  }
}
