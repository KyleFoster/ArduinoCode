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
  
  Serial.begin(115200);
  printf_begin();
  radio.begin();
  radio.enableDynamicPayloads();
  radio.setRetries(15,15);
  radio.startListening();
  radio.printDetails();
  radio.setChannel(4);
  radio.setAutoAck(true);
  //radio.openWritingPipe(pipes[1]);
  //radio.openReadingPipe(1,pipes[0]);
  //radio.startListening();
}


void loop(void) {       
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

  //Send an automatic acknoledgement of a packet being received to the sender
  radio.writeAckPayload(pipes[0], "1010", 5);
  
  // wait until message us ready to be received 
  memset(received_payload, 0, 32);
  int len = radio.getDynamicPayloadSize();
  
  // Receive the message
  radio.read(&received_payload, len);

<<<<<<< HEAD
    //convert to String and print
    String m = String(received_payload);
    Serial.print("Them: " + m.substring(0,len) + "\n\r");
  
}

=======
  //convert to String and print
  String m = String(received_payload);

  //check for ack packet 
  receiveAckPacket();
  
  Serial.print("Them: " + m.substring(0,len) + "\n\r");
  
}

void receiveAckPacket()
{
  while (!radio.available()) { }
  
  memset(ack_packet, 0, 5);
  radio.read(&ack_packet, 5);
  String ack_code = String(ack_packet);
  Serial.println("Received ack code : " + ack_code + "\n\r"); 
}

>>>>>>> 924888a4216d32857c77ea5f59ae2db5ebec0c63
