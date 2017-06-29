#include <SPI.h>
#include <stdio.h>
#include "nRF24L01.h"
#include "RF24.h"
#include "printf.h"

RF24 radio(9,10);
const uint64_t pipes[2] = {0xF0F0F0F0E1LL, 0xF0F0F0F0D2LL};

typedef enum {transmitter, receiver} mode_e;
mode_e mode;

void setup() {

  Serial.begin(57600);
  printf_begin();
  radio.begin();
  radio.setChannel(120);
  radio.setRetries(15,15);
  radio.openReadingPipe(1,pipes[1]);
  radio.startListening();
  radio.printDetails();

}

void loop() {

  Serial.flush();
  if(Serial.available()) {
    sendMessage();
  }else{ 
    receiveMessage();
  }
  
}

// ******Send a Message******

void sendMessage(void) {

  radio.stopListening();
  radio.openWritingPipe(pipes[0]);
  radio.openReadingPipe(1,pipes[1]);
  while(!Serial.available());
    String message = Serial.readString();
    Serial.print("Me: ");
    Serial.println(message);
    bool ok = radio.write(&message, sizeof(message));
    if(ok) {
      printf(" *\n\r");
    }else {
      printf("**Send Failed**\n\r");
    }
    radio.startListening();
    bool timeout;
    if(timeout) {
        printf(" *Received*\n\r");
      }else {
        printf("**Not Received**\n\r");
   }
}

// ******Receive a Message******

void receiveMessage(void) {

  radio.startListening();
  radio.openWritingPipe(pipes[1]);
  radio.openReadingPipe(1,pipes[0]);
  String message;
  bool received = radio.read(&message, sizeof(message));
  if(!received) {
    Serial.print("Them: ");
    Serial.println(message);
    delay(20);
  }
//  radio.stopListening();
//  if(received){
//    radio.write(&received, sizeof(received));
//    printf("Sent response.\n\r");
//  }
  radio.startListening();
  
}












