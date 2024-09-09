#include <ModbusMaster.h>
#include <driver/gpio_filter.h>

// initialise Modbus node

ModbusMaster node;

// Define the digital input pins for the input relays

// Define the MODBUS coil addresses for the relays
const int coilAddresses[] = {0x0000, 0x0001, 0x0002, 0x0003, 0x0004};

  // Initialize serial communication
  Serial.begin(115200);

  // Initialize serial connection for MODBUS
  Serial1.begin(9600, SERIAL_8E1, 37, 39);

  // Initialize MODBUS
  node.begin(255,Serial1);

  // Set input pins as input with internal pull-up resistors
