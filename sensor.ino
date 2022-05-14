// Project  : Scoreboard Sensor


#define CE_PIN 9                    
#define CSN_PIN 4
#define sensorPin 3

#include "RF24.h"

RF24 radio(CE_PIN, CSN_PIN);

bool previousState;
const byte address[6] = "00001";
uint8_t message 1           //0 for red, 1 for blue team.

void setup() {
                            // Setup and configure rf radio
  radio.begin();            // Start up the radio
  radio.setAutoAck(1);      // Ensure autoACK is enabled
  radio.setRetries(15, 15); // Max delay between retries.
  radio.openWritingPipe(address); // Write to device address '2Node'
  pinMode(sensorPin, INPUT_PULLUP);
}

void loop() {
  if (!digitalRead(sensorPin) && previousState) {
    radio.write(&message, sizeof(message));
    previousState = 0;
  }
  if (digitalRead(sensorPin))
    previousState = true;
}
