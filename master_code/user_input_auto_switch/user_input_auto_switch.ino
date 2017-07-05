#include <SPI.h>
#include <stdio.h>
#include "nRF24L01.h"
#include "RF24.h"
#include "printf.h"

RF24 radio(9,10);
const uint64_t pipes[2] = { 0xF0F0F0F0E1LL, 0xF0F0F0F0D2LL };
char send_payload[100] = "";
char received_payload[100] = "";


void setup(void) {
  
  Serial.begin(57600);
  printf_begin();
  radio.begin();
  radio.enableDynamicPayloads();
  radio.setRetries(15,15);
  radio.startListening();
  radio.printDetails();
  radio.setChannel(65);
  radio.setAutoAck(true);
}


void loop(void) 
{       
  receiveMessage();
}


// ***Send Message***
void sendMessage() {
  int num_of_chars = 0;

  //read in message from serial, press enter to send
  memset(send_payload, 0, 100);
  num_of_chars = Serial.readBytesUntil('\n',send_payload,100);

  //Switch roles if user inputs receive1
  String message_string = String(send_payload);

  radio.openWritingPipe(pipes[0]);
  radio.openReadingPipe(1,pipes[1]);
  radio.stopListening();
  
  radio.write(&send_payload, sizeof(send_payload));
  Serial.print("Me: " + message_string + "\n\r");
  
}

// ***Receive Message***
void receiveMessage() { 
  // wait until there is a message to receive
  while (!radio.available()) 
  {
    //check if the user wants to switch role while waiting for a message
    if (Serial.available())
    {
      sendMessage();
      radio.openWritingPipe(pipes[1]);
      radio.openReadingPipe(1,pipes[0]);
      radio.startListening();
    }  
  }
   
    memset(received_payload, 0, 32);
    int len = radio.getDynamicPayloadSize();
    
    // Receive the message
    radio.read(&received_payload, len);

    //convert to String and print
    String m = String(received_payload);
    Serial.print("Them: " + m.substring(0,len) + "\n\r");
  
}

