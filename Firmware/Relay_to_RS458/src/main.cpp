#include <ModbusMaster.h>
#include <driver/gpio_filter.h>

// initialise ModbusRTU

ModbusMaster node;

// Define the digital input pins for the input relays
const int inputPins[] = {7, 8, 9, 10, 11};

// Define the MODBUS coil addresses for the relays
const int coilAddresses[] = {0x0000, 0x0001, 0x0002, 0x0003, 0x0004};


void setup() 
  {
  // Initialize serial communication
  Serial.begin(115200);

  // Initialize serial connection for MODBUS
  Serial1.begin(9600, SERIAL_8E1, 37, 39);

  // Initialize MODBUS
  node.begin(255,Serial1);

  // Set input pins as input with internal pull-up resistors
  for (int i = 0; i < 5; i++) 
    {
      pinMode(inputPins[i], INPUT_PULLUP);
    }

  for (int i = 0; i < 5; i++) 
    {
      gpio_pin_glitch_filter_config_t config = {
        .gpio_num = (gpio_num_t)inputPins[i]
      };
      gpio_glitch_filter_handle_t filter_handle;
      gpio_glitch_filter_enable(filter_handle);
    } //end glitch filter setup

  }   // end input pin pull-up resistor routine


void loop() 
  {
    for (int i = 0; i < 5; i++) 
    {
      int inputPin = inputPins[i];
      int coilAddress = coilAddresses[i];

      bool inputValue = digitalRead(inputPin) == HIGH;
      bool valueToWrite = inputValue ? false : true;  // Explicitly map HIGH to false and LOW to true
      node.writeSingleCoil(coilAddress, valueToWrite);
    }
  }