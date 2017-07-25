#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"
#include "printf.h"
#include "EEPROM.h"
#include <string.h>

RF24 radio(9,10);

const uint64_t pipes[5] = { 0xF0F0F0F0A1LL, 0xF0F0F0F0B2LL, 0xF0F0F0F0C3LL};

                          //node 1, node 4, node 5

char input[5];

void setup() {
  Serial.begin(38400);
  printf_begin();
  radio.begin();
  radio.enableDynamicPayloads();
  radio.setRetries(15, 15);
  radio.setChannel(90);

  radio.stopListening();
  radio.openWritingPipe(pipes[0]); //open pipe to address of node 4 
}

void loop() {
  Serial.print("Enter the scene you want to start: ");
  while (!Serial.available()) { }
  if (Serial.available())
  {
    memset(input, 0, 5);
    Serial.readBytesUntil(';', input, 5);
    String scene = String(input);
    Serial.println(scene);
    if (scene == "1")
    {
      sceneOne(); 
    }
    else if (scene == "2") 
    {
      sceneTwo();
    }
    else if (scene == "3")
    {
      sceneThree();
    }
    else if (scene == "4")
    {
      sceneFour();
    }
    else if (scene == "5")
      sceneFive();
    else 
    {
      Serial.println("Retry...");
    }
  }
  
}

void sceneOne() {
  Serial.println("In scene one");
  char message[35] = "";
  memset(message, 0, 35);
  Serial.print("Enter a message: ");
  while (!Serial.available()) { }
  Serial.readBytesUntil("\n", message, 35);
  String m = String(message);
  Serial.println(m);
  radio.write("1", sizeof("1"));
  delay(10);
  radio.write(&message, sizeof(message));
}

void sceneTwo() {
  Serial.println("In scene two"); 
  //play simulation with message not being able to get over mountain
}

void sceneThree() {
  Serial.println("In scene three");  
  char message[35] = "";
  memset(message, 0, 35);
  Serial.print("Enter a message: ");
  while (!Serial.available()) { }
  Serial.readBytesUntil("\n", message, 35);
  String m = String(message);
  Serial.println(m);
  radio.write("3", sizeof("3"));
  delay(10);
  radio.write(&message, sizeof(message)); 
}

void sceneFour() {
  Serial.println("In scene four");  
  char message[35] = "";
  memset(message, 0, 35);
  Serial.print("Enter a message: ");
  while (!Serial.available()) { }
  Serial.readBytesUntil("\n", message, 35);
  String m = String(message);
  Serial.println(m);
  radio.write("4", sizeof("4"));
  delay(10);
  radio.write(&message, sizeof(message)); 
}


void sceneFive() {
  Serial.println("In scene five");  
  char message[35] = "";
  memset(message, 0, 35);
  Serial.print("Enter a message: ");
  while (!Serial.available()) { }
  Serial.readBytesUntil("\n", message, 35);
  String m = String(message);
  Serial.println(m);
  radio.write("5", sizeof("5"));
  radio.write(&message, sizeof(message)); 
}
