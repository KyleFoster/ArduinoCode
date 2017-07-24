#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"
#include "printf.h"
#include "EEPROM.h"
#include <string.h>

RF24 radio(9,10);

const uint64_t pipes[5] = { 0xF0F0F0F0E1LL, 0xF0F0F0F0D2LL, 0xF0F0F0F0C3LL,
                            0xF0F0F0F0B4L, 0xF0F0F0F0A5LL };

void setup() {
  Serial.begin(38400);
  printf_begin();
  radio.begin();
  radio.enableDynamicPayloads();
  radio.setRetries(15, 15);
  radio.startListening();
  radio.setChannel(90);

}

void loop() {
  char scene = "0";
  while (!radio.available()) { } 
  radio.read(&scene, sizeof(scene));
  if (scene == "3")
    sceneThree();
  else if (scene == "4")
    sceneFour();
  else 
    //do nothing
}

void sceneThree() {
  Serial.println("In scene three");
  char message[32] = "";
  memset(message, 0, 32);
  while (!radio.available()) { }
  radio.read(&message, sizeof(message));
  String m = String(message);
  Serial.println(m);
}


void sceneFour() {
  Serial.println("In scene four");
  char message[32] = "";
  memset(message, 0, 32);
  while (!radio.available()) { }
  radio.read(&message, sizeof(message));
  String m = String(message);
  Serial.println(m); 
}

