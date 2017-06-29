/**
 * User Input with automatic switching between transmitting and receiving nRF24L01+ radios. 
 *
 * User begins in receiver mode. To send a message, enter a message into the serial monitor. 
 * The mode is switched to transmit mode, and the message is sent out. The mode is then 
 * automatically put back into receiver mode. 
 */

#include <SPI.h>
#include <stdio.h>
#include "nRF24L01.h"
#include "RF24.h"
#include "printf.h"

//
// Hardware configuration
//

// Set up nRF24L01 radio on SPI bus plus pins 9 & 10 

RF24 radio(9,10);

//
// Topology
//

// Radio pipe addresses for the 2 nodes to communicate.
const uint64_t pipes[2] = { 0xF0F0F0F0E1LL, 0xF0F0F0F0D2LL };

//
// Role management
//
// Set up role.  This sketch uses the same software for all the nodes
// in this system.  Doing so greatly simplifies testing.  
//

// The various roles supported by this sketch
typedef enum { role_ping_out = 1, role_pong_back } role_e;

// The debug-friendly names of those roles
const char* role_friendly_name[] = { "invalid", "Ping out", "Pong back"};

// The role of the current running sketch
role_e role = role_pong_back;

char send_payload[100] = "";
char received_payload[100] = "";

void setup(void)
{
  //
  // Print preamble
  //

  Serial.begin(57600);
  printf_begin();
  printf("\n\rRF24/examples/GettingStarted/\n\r");
  printf("ROLE: %s\n\r",role_friendly_name[role]);
  printf("*** PRESS 'T' to begin transmitting to the other node\n\r");

  //
  // Setup and configure rf radio
  //

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

  //
  // Start listening
  //

  radio.startListening();

  //
  // Dump the configuration of the rf unit for debugging
  //

  radio.printDetails();
}


void loop(void)
{       
  // Pong back role.  Receive each packet, dump it out, and send it back --- RECEIVE 
  receiveMessage();
}


void sendMessage()
{
  int num_of_chars = 0;
  memset(send_payload, 0, 100);

  //read in message from serial, press enter to send
  num_of_chars = Serial.readBytesUntil('\n',send_payload,100);

  //Switch roles if user inputs receive1
  String message_string = String(send_payload);

  // Become the primary transmitter (ping out)
  role = role_ping_out;
  radio.openWritingPipe(pipes[0]);
  radio.openReadingPipe(1,pipes[1]);
  radio.stopListening();
  
  radio.write( &send_payload, sizeof(send_payload));
  printf("Message sent...\n\r");
}


void receiveMessage()
{ 
  char switch_code[100];
  bool transmit = false;
  int len = 0; 

  printf("Waiting for incoming transmissions...\n\r");

  // wait until there is a message to receive
  while ( !radio.available()) 
  {
    //check if the user wants to switch role while waiting for a message
    if (Serial.available())
    {
      sendMessage();
      transmit = true;
      printf("Returning to receive mode...\n\r");
      radio.openWritingPipe(pipes[1]);
      radio.openReadingPipe(1,pipes[0]);
      radio.startListening();
    }  
  }

  // if they dont want to switch roles, receive a message
  if (!transmit)
  { 
    // wait until message us ready to be received 
    memset(received_payload, 0, 32);
    len = radio.getDynamicPayloadSize();
    
    // Receive the message
    radio.read( &received_payload, len);

    //convert to String and print
    String m = String(received_payload);
    Serial.print("Got message: " + m.substring(0,len) + "\n\r");
  }
}
  

// vim:cin:ai:sts=2 sw=2 ft=cpp
