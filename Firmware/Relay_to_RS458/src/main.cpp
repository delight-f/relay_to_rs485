#include <ModbusMaster.h>
#include <driver/gpio_filter.h>

// ---------- Configuration ----------

// Modbus slave ID for the relay board
const uint8_t MODBUS_SLAVE_ID = 255;

// RS-485 UART settings
const unsigned long MODBUS_BAUD = 9600;
const int MODBUS_RX_PIN = 31;
const int MODBUS_TX_PIN = 35;

// Debug serial baud
const unsigned long DEBUG_BAUD = 115200;

// Number of input/coil pairs
const int NUM_CHANNELS = 5;

// Digital input pins wired to the input relays
const int inputPins[NUM_CHANNELS] = {15, 17, 18, 19, 20};

// Modbus coil addresses corresponding to each input pin
const int coilAddresses[NUM_CHANNELS] = {0x0000, 0x0001, 0x0002, 0x0003, 0x0004};

// Poll interval — sets the pace of the loop so the RS-485 bus
// isn't hammered continuously and inputs get a stable read
const unsigned long POLL_INTERVAL_MS = 50;

// Simple debounce: input must read the same value for this long
// before it's accepted as a real state change
const unsigned long DEBOUNCE_MS = 30;

// Retry attempts for a failed Modbus write
const int MAX_WRITE_RETRIES = 3;

// ---------- State ----------

ModbusMaster node;

bool lastSentState[NUM_CHANNELS];      // last value actually written to Modbus
bool lastRawState[NUM_CHANNELS];       // last raw pin reading
unsigned long lastChangeTime[NUM_CHANNELS]; // when the raw reading last changed

unsigned long lastPollTime = 0;

// ---------- Helpers ----------

// Reads the pin and applies the active-low mapping:
// pull-up means HIGH = relay not active (false), LOW = relay active (true)
bool readMappedInput(int pin) {
  return digitalRead(pin) == LOW;
}

// Writes a coil with retries. Returns true on success.
bool writeCoilWithRetry(uint16_t address, bool value, int channelIndex) {
  for (int attempt = 1; attempt <= MAX_WRITE_RETRIES; attempt++) {
    uint8_t result = node.writeSingleCoil(address, value);
    if (result == node.ku8MBSuccess) {
      return true;
    }
    Serial.printf("Channel %d: coil 0x%04X write failed (code 0x%02X), attempt %d/%d\n",
                  channelIndex, address, result, attempt, MAX_WRITE_RETRIES);
    delay(10); // brief pause before retrying, avoids re-flooding an already-busy bus
  }
  Serial.printf("Channel %d: coil 0x%04X write abandoned after %d attempts\n",
                channelIndex, address, MAX_WRITE_RETRIES);
  return false;
}

// ---------- Setup ----------

void setup() {
  Serial.begin(DEBUG_BAUD);

  Serial1.begin(MODBUS_BAUD, SERIAL_8E1, MODBUS_RX_PIN, MODBUS_TX_PIN);
  node.begin(MODBUS_SLAVE_ID, Serial1);

  for (int i = 0; i < NUM_CHANNELS; i++) {
    pinMode(inputPins[i], INPUT_PULLUP);
  }

  // Configure and enable a glitch filter on each input pin.
  // Each pin needs its own config struct and its own handle —
  // the handle is populated by gpio_new_pin_glitch_filter, not
  // declared empty and enabled directly.
  for (int i = 0; i < NUM_CHANNELS; i++) {
    gpio_pin_glitch_filter_config_t filterConfig = {
      .gpio_num = (gpio_num_t)inputPins[i]
    };

    gpio_glitch_filter_handle_t filterHandle = NULL;
    esp_err_t err = gpio_new_pin_glitch_filter(&filterConfig, &filterHandle);
    if (err != ESP_OK) {
      Serial.printf("Pin %d: failed to create glitch filter (err %d)\n", inputPins[i], err);
      continue;
    }

    err = gpio_glitch_filter_enable(filterHandle);
    if (err != ESP_OK) {
      Serial.printf("Pin %d: failed to enable glitch filter (err %d)\n", inputPins[i], err);
    }
  }

  // Prime state so the first loop iteration only sends coils
  // that genuinely need setting, rather than assuming a stale default
  for (int i = 0; i < NUM_CHANNELS; i++) {
    bool initial = readMappedInput(inputPins[i]);
    lastRawState[i] = initial;
    lastChangeTime[i] = millis();

    if (writeCoilWithRetry(coilAddresses[i], initial, i)) {
      lastSentState[i] = initial;
    } else {
      // If the initial write fails, force a resend attempt on the
      // first loop pass by marking the sent state as unknown/opposite
      lastSentState[i] = !initial;
    }
  }
}

// ---------- Loop ----------

void loop() {
  unsigned long now = millis();

  if (now - lastPollTime < POLL_INTERVAL_MS) {
    return; // not time to poll yet — keeps bus traffic paced
  }
  lastPollTime = now;

  for (int i = 0; i < NUM_CHANNELS; i++) {
    bool raw = readMappedInput(inputPins[i]);

    if (raw != lastRawState[i]) {
      // Raw reading changed — restart the debounce timer
      lastRawState[i] = raw;
      lastChangeTime[i] = now;
      continue;
    }

    // Raw reading has been stable — check if it's been stable long enough
    if ((now - lastChangeTime[i]) < DEBOUNCE_MS) {
      continue;
    }

    // Debounced value differs from what was last sent — send it
    if (raw != lastSentState[i]) {
      if (writeCoilWithRetry(coilAddresses[i], raw, i)) {
        lastSentState[i] = raw;
      }
      // on failure, lastSentState is left unchanged so it will retry next poll
    }
  }
}
