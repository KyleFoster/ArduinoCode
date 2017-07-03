#include <SPI.h>
#include <stdio.h>
#include "nRF24L01.h"
#include "RF24.h"
#include "printf.h"

RF24 radio(9,10);

const uint64_t pipes[2] = { 0xF0F0F0F0E1LL, 0xF0F0F0F0D2LL };

typedef enum { role_ping_out = 1, role_pong_back } role_e;

// The debug-friendly names of those roles
const char* role_friendly_name[] = { "invalid", "Ping out", "Pong back"};

// The role of the current running sketch
role_e role = role_pong_back;

char send_payload[100] = "";
char received_payload[100] = "";

void setup(void)
{
 
  Serial.begin(57600);
  printf_begin();
  printf("\n\rRF24/examples/GettingStarted/\n\r");
  printf("ROLE: %s\n\r",role_friendly_name[role]);
  printf("*** PRESS 'T' to begin transmitting to the other node\n\r");

  radio.begin();

  // optionally, increase the delay between retries & # of retries
  radio.enableDynamicPayloads();
  radio.setRetries(15,15);

  // optionally, reduce the payload size.  seems to
  // improve reliability
  //radio.setPayloadSize(8);

  //
  // Open pipes to other nodes for communication
  //

  // This simple sketch opens two pipes for these two nodes to communicate
  // back and forth.
  // Open 'our' pipe for writing
  // Open the 'other' pipe for reading, in position #1 (we can have up to 5 pipes open for reading)

  if ( role == role_ping_out )
  {
    radio.openWritingPipe(pipes[0]);
    radio.openReadingPipe(1,pipes[1]);
  }
  else
  {
    radio.openWritingPipe(pipes[1]);
    radio.openReadingPipe(1,pipes[0]);
  }

  radio.startListening();
  radio.printDetails();
}


void loop(void)
{ 
  // Ping out role.  Repeatedly send the current time  --- TRANSMISSION

  if (role == role_ping_out)
  {
    // Stop listening so you can send message
    radio.stopListening();
    sendMessage();
  }
       
  // Pong back role.  Receive each packet, dump it out, and send it back --- RECEIVE 

  if ( role == role_pong_back )
  {
    receiveMessage();
  }
}


void switchRole(String c)
{
  if (c == "t" && role == role_pong_back)
  {
    printf("*** CHANGING TO TRANSMIT ROLE -- ENTER 'receive1' TO SWITCH BACK\n\r");

    // Become the primary transmitter (ping out)
    role = role_ping_out;
    radio.openWritingPipe(pipes[0]);
    radio.openReadingPipe(1,pipes[1]);
  }
  else if (c == "r"  && role == role_ping_out)
  {
    printf("*** CHANGING TO RECEIVE ROLE -- ENTER 'transmit1' TO SWITCH BACK\n\r");
      
    // Become the primary receiver (pong back)
    role = role_pong_back;
    radio.openWritingPipe(pipes[1]);
    radio.openReadingPipe(1,pipes[0]);
    radio.startListening();
  } 
}


void sendMessage()
{
  int num_of_chars = 0;
  //char message_buffer[100];

  printf("Enter a message to transmit...\n\r");
  
  // wait for user to write message to begin transmission
  while (!Serial.available()) { }
  
  //read in message from serial, press enter to send
  num_of_chars = Serial.readBytesUntil('\n',send_payload,100);

  //Switch roles if user inputs receive1
  String message_string = String(send_payload);
  if (message_string.substring(0,8) == "receive1" || message_string.substring(0,9) == "transmit1")
    switchRole(message_string.substring(0,1));
  else //else send the message
  {
    //send the text message
    radio.write( &send_payload, sizeof(send_payload));
  }
}


void receiveMessage()
{
  bool message_received = false;
  bool switch_role = false; 
  int num_of_chars_received = 0;
  char switch_code[10];

  printf("Waiting for incoming transmissions...\n\r");

  // wait until there is a message to receive
  while ( !radio.available()) 
  {
    //check if the user wants to switch role while waiting for a message
    if (Serial.available())
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

  // if they dont want to switch roles, receive a message
  if (!switch_role)
  { 
    // wait until message us ready to be received 
    int len = radio.getDynamicPayloadSize();
    
    // Receive the message
    radio.read( &received_payload, len);

    //convert to String and print
    String m = String(received_payload);
    Serial.print("Got message: " + m.substring(0,len) + "\n\r");
  }
}
  
