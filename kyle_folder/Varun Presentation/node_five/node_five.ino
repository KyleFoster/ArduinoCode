#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"
#include "printf.h"
#include "EEPROM.h"
#include <string.h>

RF24 radio(9,10);

const uint64_t pipes[5] = { 0xF0F0F0F0A1LL, 0xF0F0F0F0B2LL, 0xF0F0F0F0C3LL};

void setup() {
  Serial.begin(38400);
  printf_begin();
  radio.begin();
  radio.enableDynamicPayloads();
  radio.setRetries(15, 15);
  radio.setChannel(90);

  radio.openReadingPipe(0, pipes[2]);
  radio.openReadingPipe(1, pipes[1]);
  radio.openReadingPipe(2, pipes[0]);
  radio.startListening();
}

void loop() {
  char scene = '0';
  char trash[32] = "";
  while (!radio.available()) { } 
  radio.read(&scene, sizeof(scene));
  if (scene == '3')
    sceneThree();
  else if (scene == '4')
    sceneFour();
  else if (scene == '5')
    sceneFive();
  else 
  {
    radio.read(&trash, sizeof(trash));
    String t = String(t);
    Serial.println("Trash: " + t);
  }
}

void sceneThree() {
  Serial.println("In scene three");
  String m = receiveMessage();
  printf("Node1 -> Node4 -> Node5\n\r");
  Serial.println("Message: " + m);
}


void sceneFour() {
  Serial.println("In scene four");
  String m = receiveMessage();
  printf("Node1 -> Node2 -> Node3 -> Node5\n\r");
  Serial.println("Message: " + m);
}


void sceneFive() {
  Serial.println("In scene five");
  String m = receiveMessage();
  printf("Node1 -> Node4 -> Node3 -> Node5\n\r");
  Serial.println("Message: " + m);
}

String receiveMessage(){
  char message[32] = "";
  memset(message, 0, 32);
  while (!radio.available()) { } 
  radio.read(&message, sizeof(message));
  String m = String(message);
  return m; 
}

