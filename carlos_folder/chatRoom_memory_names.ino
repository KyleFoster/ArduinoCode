#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"
#include "printf.h"
#include "EEPROM.h"
#include <string.h>

RF24 radio(9,10);
const uint64_t pipes[2] = { 0xF0F0F0F0E1LL, 0xF0F0F0F0D2LL };
char send_payload[100] = "";
char received_payload[100] = "";

int power, rate, channel;
int nameLength;
int nameLengthAddr = 11;
int nameBegin =  0;
int powerAddr = 12;
int rateAddr = 13;
int channelAddr = 14;
String myName;

void setup() {

  Serial.begin(9600);
  printf_begin();
  radio.begin();
  radio.enableDynamicPayloads();
  radio.setRetries(15,15);
  radio.startListening();
  radio.setAutoAck(true);

  nameLength = EEPROM.read(nameLengthAddr);
  char names[nameLength];
  for(int i = nameBegin; i <= nameLength; i++) {
    names[i] = EEPROM.read(i);
  }
  myName = getMyName(names);
  String name1 = String(names);
  Serial.print("This Arduino belongs to " + name1 + ".\n\r");
  Serial.print("Load " + name1 + "'s profile? Type 'Y' for Yes or any key to create a new profile.\n\r");
  Serial.flush();
  while(!Serial.available()) { };
  String load = Serial.readString();
  load.toUpperCase();
  if (load == "Y") 
  {
    power = EEPROM.read(powerAddr); //Recall saved parameters and skip setup dialogs
      choosePower(power);
    rate = EEPROM.read(rateAddr);
      setRate(rate);
    channel = EEPROM.read(channelAddr);
      setChannel(channel);
  }
  else 
  {
    Serial.print("Enter a new name: ");
    Serial.flush();
    while(!Serial.available());
    String newName = Serial.readString(); 
    nameLength = eepromWrite(nameBegin, newName);
    Serial.println(newName);

    Serial.println("Choose TX Power Level:\n\r 0: -18dBm\n\r 1: -12dBm\n\r 2: -6dBm \n\r 3: 0dBm  \n\r");
    Serial.flush(); //Clear Serial buffer
    while(!Serial.available()); //Wait until something is being sent to the Arduino from the Serial monitor
    power = Serial.parseInt();
    choosePower(power);
    EEPROM.write(powerAddr, power);

    Serial.println("Choose Data Rate:\n\r 0: 250kbps\n\r 1: 1Mbps\n\r 2: 2Mbps\n\r");
    Serial.flush();
    while(!Serial.available());
    rate = Serial.parseInt();
    setRate(rate);
    EEPROM.write(rateAddr, rate); 

    Serial.println("Set Channel: Enter 0 - 127 or type 76 for default channel");
    Serial.flush();
    while(!Serial.available());
    channel = Serial.parseInt();
    float frequency = ((2400 + channel) * 0.001);
    Serial.print("Channel set to ");
    Serial.print(channel);
    Serial.print(". Frequency = ");
    Serial.print((float)(frequency),3);
    Serial.println("GHz\n\r");
    setChannel(channel);
    EEPROM.write(channelAddr, channel);
    
  }
  radio.printDetails();
}

void loop(void) 
{       
  receiveMessage();
}

// ***Send Message***
void sendMessage() 
{
  //Serial.print(myName);
  char message[120] = "";
 //strcat(message, myName);
  strcat(message, ": ");

  //read in message from serial, press enter to send
  memset(send_payload, 0, 100);
  Serial.readBytesUntil('\n',send_payload,100);

  strcat(message, send_payload);
  String str1 = String(message);
  Serial.println(myName + str1);

  radio.openWritingPipe(pipes[0]);
  radio.openReadingPipe(1,pipes[1]);
  radio.stopListening();

  radio.write(&message, sizeof(message));
  //Serial.println(*myName);
}

// ***Receive Message***
void receiveMessage() { 
  int num_of_chars_received = 0;

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
  
    // wait until message is ready to be received 
    memset(received_payload, 0, 32);
    int len = radio.getDynamicPayloadSize();
    
    // Receive the message
    radio.read(&received_payload, len);

    //convert to String and print
    String m = String(received_payload);
    Serial.print(m.substring(0,len) + "\n\r");
  
}

// ***Write to EEPROM***
int eepromWrite(int startAddress, String string) {
  int strLength = string.length() + 1;
  char charBuff[strLength];
  string.toCharArray(charBuff, strLength);
  for(int i = startAddress; i <= strLength; i++) {
    EEPROM.write(i, charBuff[i]);
  }
  EEPROM.write(nameLengthAddr, strLength);
  return strLength;
}

// ***Read from EEPROM***
String eepromRead(int startAddress, int strLength) {
  char charBuff[strLength];
  for(int i = startAddress; i <= strLength; i++) {
    charBuff[i] = EEPROM.read(i);
  }
  return charBuff;  
}

// ***EEPROM Clear***
void eepromClear(int startAddress, int strLength) 
{
  for (int i = 0; i <= strLength; i++) {
     EEPROM.write(i, 0);
   }
}

// ***Choose Power***
void choosePower(int POWER)  //Select which power level to run the nRF24L01+ at
{
    if(POWER == 0) {
      radio.setPALevel(RF24_PA_MIN);
    }else if(POWER == 1) {
      radio.setPALevel(RF24_PA_LOW);
    }else if(POWER == 2) {
      radio.setPALevel(RF24_PA_HIGH);
    }else if(POWER == 3) {
      radio.setPALevel(RF24_PA_MAX);
    }
}

// ***Set Data Rate***
void setRate(int RATE) //Select data rate to jam specific communication protocols
{ 
  if(RATE == 0) {
    radio.setDataRate(RF24_250KBPS);
  }else if(RATE == 1) {
    radio.setDataRate(RF24_1MBPS);
  }else if(RATE == 2) {
    radio.setDataRate(RF24_2MBPS);
  }
}

// ***Set Channel Frequency***
void setChannel(int CHANNEL) 
{
  radio.setChannel(CHANNEL);  
}

// ***Get My Name to be Globally available***

String getMyName(String myName)
{
  String x = myName;
  return x;
}

