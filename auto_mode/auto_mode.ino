#include <SPI.h>
#include <stdio.h>
#include "nRF24L01.h"
#include "RF24.h"
#include "printf.h"

RF24 radio(9,10);
const uint64_t pipes[2] = {0xF0F0F0F0E1LL, 0xF0F0F0F0D2LL};
typedef enum { role_ping_out = 1, role_pong_back } role_e;
const char* role_friendly_name[] = { "invalid", "Ping out", "Pong back"};
role_e role = role_pong_back;

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
  if(!Serial.available()) {
    radio.stopListening();
    receiveMessage();
  }else{
    radio.startListening(); 
    receiveMessage();
  }
  
}

// ***********************************************************************
void switchRole(String c)
{
  if (c == "t" && role == role_pong_back)
  {
    printf("*** CHANGING TO TRANSMIT ROLE -- ENTER 'receive1' TO SWITCH BACK\n\r");

    // Become the primary transmitter (ping out)
    radio.stopListening();
    role = role_ping_out;
    radio.openWritingPipe(pipes[0]);
    radio.openReadingPipe(1,pipes[1]);
  }
  else if (c == "r"  && role == role_ping_out)
  {
    printf("*** CHANGING TO RECEIVE ROLE -- ENTER 'transmit1' TO SWITCH BACK\n\r");
      
    // Become the primary receiver (pong back)
    radio.startListening();
    role = role_pong_back;
    radio.openWritingPipe(pipes[1]);
    radio.openReadingPipe(1,pipes[0]);
  } 
}

// ******************************************************************
void receiveMessage()
{
  bool message_received = false;
  bool switch_role = false; 
  int num_of_chars_received = 0;
  char switch_code[10];

  printf("Waiting for incoming transmissions...\n\r");

  // wait until there is a message to receive
  while ( radio.available() == false) 
  {
    //check if the user wants to switch role while waiting for a message
    if (Serial.available() > 0)
    {
      Serial.readBytesUntil('\n', switch_code, 10);
      String s = String(switch_code);
      if (s.substring(0,8) == "receive1" || s.substring(0,9) == "transmit1")
      {
        switchRole(s.substring(0,1)); 
        switch_role = true;
        break;
      }
    }  
  }
}
  
// **********************************************************************
void sendMessage()
{
  int num_of_chars = 0;
  char message_buffer[100];

  printf("Enter a message to transmit...\n\r");
  
  // wait for user to write message to begin transmission
  while (Serial.available() == 0) { }
  
  //read in message from serial, press enter to send
  num_of_chars = Serial.readBytesUntil('\n',message_buffer,100);

  //Switch roles if user inputs receive1
  String message_string = String(message_buffer);
  if (message_string.substring(0,8) == "receive1" || message_string.substring(0,9) == "transmit1")
    switchRole(message_string.substring(0,1));
  else //else send the message
  {
    bool length_sent = false;
    bool message_sent = false;
      
    // send length of incoming message to receiver
    length_sent = radio.write( &num_of_chars, sizeof(int));

    // if length sent, send the text message
    if (length_sent)
    {
      message_sent = radio.write( &message_buffer, num_of_chars);
      if (message_sent)
        printf("Message sent...\n\r");
      else
        printf("Failed to send message...\n\r");
    }
    else
      printf("Failed to send length...\n\r");
  }  
}


