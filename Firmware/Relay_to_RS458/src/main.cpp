#include <ArduinoRS485.h>
#include <ArduinoModbus.h>

// Define the digital input pins for the input relays
const int inputPins[5] = {2, 3, 4, 5, 6};

// Define the MODBUS coil addresses for the relays
const int coilAddresses[5] = {0x0000, 0x0001, 0x0002, 0x0003, 0x0004};

void setup() {
  // Initialize serial communication
  Serial.begin(9600);
  
  ModbusRTUClient.begin(9600);
  
   // Set input pins as input with internal pull-up resistors
  for (int i = 0; i < 5; i++) {
    pinMode(inputPins[i], INPUT_PULLUP);

  }
   }

void loop() {
  // Check the state of each relay pin
  for (int i = 0; i < 5; i++) {
    if (digitalRead(inputPins[i]) == LOW) {
      // Input is LOW, send MODBUS command to close the corresponding relay
      ModbusRTUClient.coilWrite(0xFF,coilAddresses[i], 0x00);
    } else {
      // Input is HIGH, send MODBUS command to open the corresponding relay
      ModbusRTUClient.coilWrite(0xFF,coilAddresses[i], 0xFF);
    }
  }
  millis(100); // Small delay to avoid excessive polling
}
  

