#include "SPI.h"
#include "stdio.h"
#include "nRF24L01.h"
#include "RF24.h"
#include "printf.h"
#include "EEPROM.h"

RF24 radio(9,10);
const uint64_t pipes[2] = { 0xF0F0F0F0E1LL, 0xF0F0F0F0D2LL };
char send_payload[100] = "";
char received_payload[100] = "";

String returned;
int x;

void setup(void) {
  
  Serial.begin(57600);
  printf_begin();
  radio.begin();

  printf("Save?\n\r");
  Serial.flush();
  while(!Serial.available());
  String save = Serial.readString(); 
  save.toUpperCase(); //Convert input to uppercase to guarantee a match
  if(save == "Y") { 
  printf("Enter your name\n\r");
  Serial.flush();
  while(!Serial.available());
  String names = Serial.readString();
  x = eepromWrite(names, 0);
  returned = eepromRead(0, x);
  Serial.println(returned);
  }else {
  returned = eepromRead(0, x);
  Serial.println(returned);
  }
  
  radio.enableDynamicPayloads();
  radio.setRetries(15,15);
  radio.startListening();
  radio.printDetails();
  
}


void loop(void) {       
  receiveMessage();
}


// ***Send Message***
void sendMessage() {
  int num_of_chars = 0;
  String myName = (

  //read in message from serial, press enter to send
  memset(send_payload, 0, 100);
  num_of_chars = Serial.readBytesUntil('\n',send_payload,100);
  
  
  String message_string = String(send_payload);

  radio.openWritingPipe(pipes[0]);
  radio.openReadingPipe(1,pipes[1]);
  radio.stopListening();
  
  radio.write(&send_payload, sizeof(send_payload));
  Serial.print("Me: ");
  Serial.println(message_string);
}

// ***Receive Message***
void receiveMessage() { 
  int num_of_chars_received = 0;
  char switch_code[100];

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
  
    // wait until message us ready to be received 
    memset(received_payload, 0, 32);
    int len = radio.getDynamicPayloadSize();
    
    // Receive the message
    radio.read(&received_payload, len);

    //convert to String and print
    String m = String(received_payload);
    Serial.print("Them: " + m.substring(0,len) + "\n\r");
  
}


// ***EEPROM Write String***
int eepromWrite(String str, int startAddr) {
  for(int i = startAddr; i < str.length(); i++) {
    EEPROM.write(i,str[i]);
  }
  return str.length();
}

// ***EEPROM Read String***
String eepromRead(int startAddr, int y) {
  char str[y];
  for(int i = startAddr; i <= y; i++) {
    str[i] = EEPROM.read(i);
  }
  return str;  
}


