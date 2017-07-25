#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"
#include "printf.h"
#include "EEPROM.h"
#include <string.h>

RF24 radio(9,10);

const uint64_t pipes[3] = { 0xF0F0F0F0A1LL, 0xF0F0F0F0B2LL, 0xF0F0F0F0C3LL};

void setup() {
  Serial.begin(38400);
  printf_begin();
  radio.begin();
  radio.enableDynamicPayloads();
  radio.setRetries(15, 15);
  radio.setChannel(90);

  radio.openReadingPipe(0, pipes[0]);
  radio.openReadingPipe(1, pipes[1]);
  radio.openReadingPipe(2, pipes[2]);
  radio.startListening();
}

void loop() {
  char scene = '0';
  while (!radio.available()) { } 
  radio.read(&scene, sizeof(scene));
  if (scene == '1')
    sceneOne();
  else 
    int x = 5;
    //do nothing
}

void sceneOne() {
  Serial.println("In scene One");
  char message[32] = "";
  memset(message, 0, 32);
  while (!radio.available()) { } 
  radio.read(&message, sizeof(message));
  String m = String(message);
  printf("Node1 -> Node4\n\r");
  Serial.println("Message: " + m);
}


