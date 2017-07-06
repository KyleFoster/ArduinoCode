#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"
#include "printf.h"

RF24 radio(9,10);
const uint64_t pipes[2] = { 0xF0F0F0F0E1LL, 0xF0F0F0F0D2LL };
char ack[100] = "";
char send_code[5] = "1010";
char send_payload[100] = "";
char received_payload[100] = "";
char broadcast_message[5] = "1234";
char user_input[5] = "";
bool search_channels = true;



void setup(void)
{
  Serial.begin(57600);
  printf_begin();
  printf("\n\rRF24/examples/scanner/\n\r");
  radio.begin();
  radio.enableDynamicPayloads();
  radio.enableAckPayload();
  radio.printDetails();
}

void loop(void)
{
  Serial.println("Enter 1 to set your own channel, Enter 2 to search: ");

  while (!Serial.available()) { }
  Serial.readBytesUntil('\n',user_input,5);
  String user_string = String(user_input);

  if (user_string == "1")
  {
    Serial.println("Set your channel 0-127: ");
    while (!Serial.available()) { }
    Serial.readBytesUntil('\n',user_input,5);
    user_string = String(user_input);
    Serial.println("Setting channel to " + user_string);
    int c = user_string.toInt();
    radio.setChannel(c);
    receiveMessage(); 
  }
  else
  {
    if (search_channels)
      findChannel();
    receiveMessage(); 
  }  
}


void findChannel() {
  Serial.println("searching for channels...");
  bool found_channel = false;
  int i = 127;
  
  while (!found_channel)
  {
    radio.setChannel(i);
    broadcast();
    //Serial.println("On channel: " + i);
    if ( radio.testCarrier())
    {
      printf("Sending message on channel: %d \n\r", i);
      sendCode();
      receiveAutoAck();
      if (checkCode())
        found_channel = true;
      else
        Serial.println("Failed to authenticate");
    }
    i--;
    if (i<0)
      i = 127;
  }
}


// ***Send Message***
void sendCode() {
  radio.openWritingPipe(pipes[0]);
  radio.openReadingPipe(1,pipes[1]);
  radio.stopListening();
  
  radio.write(&send_code, sizeof(send_code));
}


void receiveAutoAck() {
  radio.openWritingPipe(pipes[1]);
  radio.openReadingPipe(1,pipes[0]);
  radio.startListening();

  memset(ack, 0, 32);
  int len = radio.getDynamicPayloadSize();
    
  // Receive the message
  radio.read(&ack, len);
}


bool checkCode() {
  bool correct_code = false;
  String code = String(ack);
  
  if (code == "1010")
    correct_code = true;
  return correct_code;
}


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
  if (radio.isAckPayloadAvailable())
    receiveAutoAck();  
  Serial.print("Me: " + message_string + "\n\r"); 
}


// ***Receive Message***
void receiveMessage() { 
  // wait until there is a message to receive
  while (!radio.available()) 
  {
    //Broadcast a message for other radios to pick up
    broadcast();
    radio.openWritingPipe(pipes[1]);
    radio.openReadingPipe(1,pipes[0]);
    radio.startListening();
  
    //check if the user wants to switch role while waiting for a message
    if (Serial.available())
    {
      sendMessage();
      radio.openWritingPipe(pipes[1]);
      radio.openReadingPipe(1,pipes[0]);
      radio.startListening();
    }  
  }

  radio.writeAckPayload(pipes[0], "1010", 5);
   
  memset(received_payload, 0, 32);
  int len = radio.getDynamicPayloadSize();
  
  // Receive the message
  radio.read(&received_payload, len);

  //convert to String and print
  String m = String(received_payload);

  if (m != "1234")
    Serial.print("Them: " + m.substring(0,len) + "\n\r");
}


void broadcast() {
  radio.openWritingPipe(pipes[0]);
  radio.openReadingPipe(1,pipes[1]);
  radio.stopListening();
  
  radio.write(&broadcast_message, 5);
  if (radio.isAckPayloadAvailable())
    receiveAutoAck();  
}





