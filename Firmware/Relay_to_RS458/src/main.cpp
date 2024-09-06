#include <ModbusMaster.h>
#include <driver/gpio_filter.h>
#include <driver/gpio.h>

/* Modbus stuff */
#define MODBUS_DIR_PIN  20 // connect DR, RE pin of MAX485 to gpio 20
#define MODBUS_RX_PIN 16// Rx pin  
#define MODBUS_TX_PIN 17 // Tx pin 
#define MODBUS_SERIAL_BAUD 9600 // Baud rate for esp32 and max485 communication

//Initialize the ModbusMaster object as node
ModbusMaster node;

// Pin 20 made high for Modbus transmision mode
void modbusPreTransmission()
{
  delay(3);
  digitalWrite(MODBUS_DIR_PIN, HIGH);
}
// Pin 20 made low for Modbus receive mode
void modbusPostTransmission()
{
  digitalWrite(MODBUS_DIR_PIN, LOW);
  delay(3);
}

// Define the digital input pins for the input relays
const int inputPins[5] = {2, 3, 4, 5, 6};

// Define the MODBUS coil addresses for the relays
const int coilAddresses[5] = {0x0000, 0x0001, 0x0002, 0x0003, 0x0004};

void setup() 
  {
    // Initialize serial communication
    Serial.begin(115200);

    pinMode(MODBUS_DIR_PIN, OUTPUT);
    digitalWrite(MODBUS_DIR_PIN, LOW);

    //Serial0.begin(baud-rate, protocol, RX pin, TX pin);.
    Serial0.begin(MODBUS_SERIAL_BAUD, SERIAL_8E1, MODBUS_TX_PIN, MODBUS_RX_PIN);
    Serial0.setTimeout(200);
    //modbus slave ID 255
    node.begin(255, Serial0);
    
    // Set input pins as input with internal pull-up resistors
    for (int pin : inputPins) {
    pinMode(pin, INPUT_PULLUP);

}   // end input pin pull-up resistor routine

// create glitch filters

for (int i = 2; i <= 6; i++) {
  gpio_pin_glitch_filter_config_t config = {
    .gpio_num = (gpio_num_t)i
  };
  gpio_glitch_filter_handle_t filter_handle;
  gpio_glitch_filter_enable(filter_handle);
} //end glitch filter setup

//  callbacks allow us to configure the RS485 transceiver correctly  
    node.preTransmission(modbusPreTransmission);
    node.postTransmission(modbusPostTransmission);
    
} // end initial setup
 void loop() 
{
  for (int i = 0; i < 5; i++) {
    if (digitalRead(inputPins[i]) == HIGH) {
      node.writeSingleCoil(coilAddresses[i], 0xFF);
    } else {
      node.writeSingleCoil(coilAddresses[i], 0x00);
    }
  }
  delay(100); // Small delay to avoid excessive polling
}