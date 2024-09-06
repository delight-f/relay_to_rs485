#include <ModBusRTU.h>
#include <ModbusAPI.h>
#include <driver/gpio_filter.h>
#include <driver/gpio.h>

// initialise ModbusRTU

ModbusRTU mb;

// Define the digital input pins for the input relays
const int inputPins[5] = {2, 4, 5, 6, 7};

// Define the MODBUS coil addresses for the relays
const int coilAddresses[5] = {0x0000, 0x0001, 0x0002, 0x0003, 0x0004};

void setup() 
  {
    // Initialize serial communication
    Serial.begin(115200);

    //Serial1.begin(baud-rate, protocol, RX pin, TX pin);.
    Serial1.begin(9600,SERIAL_8E1, 37, 39);
    Serial1.setTimeout(200);

    //modbusRTU begin
    mb.begin(&Serial1);
    mb.client();
     
    // Set input pins as input with internal pull-up resistors
    for (int pin : inputPins) {
    pinMode(pin, INPUT_PULLUP);


}   // end input pin pull-up resistor routine

} // end initial setup
 void loop() {
  for (int i = 0; i < 5; i++) {
    if (digitalRead(inputPins[i]) == HIGH) {
      mb.writeCoil(0xFF, coilAddresses[i], true);
    } else {
      mb.writeCoil(0xFF, coilAddresses[i], false);
    }
    mb.task(); // Call mb.task() after each write operation
  }
  yield;
}
