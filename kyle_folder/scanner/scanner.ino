#include <SPI.h>
#include <stdio.h>
#include "nRF24L01.h"
#include "RF24.h"
#include "printf.h"

RF24 radio(9,10);
const uint64_t pipes[2] = { 0xF0F0F0F0E1LL, 0xF0F0F0F0D2LL };
char send_payload[100] = "";
char received_payload[100] = "";
int channel = 0;

void setup(void) 
{
  
  Serial.begin(57600);
  printf_begin();
  radio.begin();
  radio.enableDynamicPayloads();
  radio.setRetries(15,15);
  radio.printDetails();
}


void loop(void) 
{  
  printf("channel %d...\n\r", channel); 
  delay(100);
  radio.setChannel(channel);   
  //if (radio.testCarrier())
  //{    
  //printf("test carrier\n\r");
  if (sendMessage())
  {
    delay(100);
    if (checkMessage())
    {
      printf("Staying on channel %d...\n\r", channel); 
    }
  }
  //}
  channel++;
  if (channel >= 6)
    channel = 0;
}


// ***Send Message***
bool sendMessage() 
{ 
  printf("sending message...\n\r");
  bool sent = false;
  
  radio.openWritingPipe(pipes[0]);
  radio.openReadingPipe(1,pipes[1]);
  radio.stopListening();

  char code[5] = "1010";
  
  sent = radio.write(&code, sizeof(code));
  
  return sent ;
}

// ***Receive Message***
bool checkMessage() 
{ 
  radio.openWritingPipe(pipes[1]);
  radio.openReadingPipe(1,pipes[0]);
  radio.startListening();
  
  printf("checking message...\n\r");
  bool correct_message = false;
  
  // wait until there is a message to receive
  int start_time = millis();
  int five_second_interval = millis();

  while (start_time + 5000 > five_second_interval)
  {  
    if (radio.available())
    {
      memset(received_payload, 0, 32);
      int len = radio.getDynamicPayloadSize();
        
      // Receive the message
      radio.read(&received_payload, len);
    
      //convert to String and print
      String m = String(received_payload);
    
      if (m.substring(0,3) == "1010")
        correct_message = true; 
    }
    five_second_interval = millis();
  }
  
  return correct_message;
}

