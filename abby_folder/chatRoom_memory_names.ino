#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"
#include "printf.h"
#include "EEPROM.h"
#include <string.h>

RF24 radio(9,10);
const uint8_t pipes[2] = {0xE8, 0xD8};
char send_payload[100] = "";
char received_payload[100] = "";
char names[10];

int power, rate, channel;
int nameLength = 10;
int nameBegin =  0;
int nameLengthAddr = 11;
int powerAddr = 12;
int rateAddr = 13;
int channelAddr = 14;
String myName;

struct RadioHeader {
  uint8_t to_address;
  uint8_t from_address;
} myHeader={pipes[0], pipes[1]};

void setup() {

  Serial.begin(9600);
  printf_begin();
  radio.begin();
  radio.enableDynamicPayloads();
  radio.setRetries(15, 15);
  radio.startListening();
  radio.setAutoAck(true);

  //nameLength = EEPROM.read(nameLengthAddr);
  //char names[nameLength];
  for(int i = nameBegin; i <= nameLength; i++) {
    names[i] = EEPROM.read(i);
  }
  myName = getMyName(names);
  Serial.print("This Arduino belongs to " + myName + ".\n\r");
  Serial.print("Load " + myName + "'s profile? Type 'Y' for Yes or any key to create a new profile.\n\r");
  Serial.flush();
  while(!Serial.available());
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
    Serial.print("Enter a new name. Max 10 characters: ");
    Serial.flush();
    while(!Serial.available()); //Wait until something is being sent to the Arduino from the Serial monitor
    String newName = Serial.readString(); 
    nameLength = eepromWrite(nameBegin, newName);
    Serial.println(newName);

    Serial.println("Choose TX Power Level:\n\r 0: -18dBm\n\r 1: -12dBm\n\r 2: -6dBm \n\r 3: 0dBm  \n\r");
    Serial.flush(); //Clear Serial buffer
    while(!Serial.available());
    power = Serial.parseInt();
    choosePower(power);
    EEPROM.write(powerAddr, power);

    Serial.println("Choose Data Rate:\n\r 0: 250kbps\n\r 1: 1Mbps\n\r 2: 2Mbps\n\r");
    Serial.flush();
    while(!Serial.available());
    rate = Serial.parseInt();
    setRate(rate);
    EEPROM.write(rateAddr, rate); 

    Serial.println("Set Channel: Enter 0 - 127 or 76 for default");
    Serial.flush();
    while(!Serial.available());
    channel = Serial.parseInt();
    float frequency = ((2400 + channel) * 0.001);
    Serial.print("Channel set to ");
    Serial.print(channel);
    Serial.print(". Frequency = ");
    Serial.print((float)(frequency),3);
    Serial.println(" GHz\n\r");
    setChannel(channel);
    EEPROM.write(channelAddr, channel);

    myName=eepromRead(nameBegin, nameLength);
    
  }
  radio.printDetails();
  Serial.println("---------------Begin Chat!---------------\n\r");
}

void loop(void) 
{       
  receiveMessage();
}

// ***Send Message***
void sendMessage() 
{
  memset(send_payload, 0, 100);
  Serial.readBytesUntil('\n',send_payload,100);
 
  radio.openWritingPipe(pipes[0]);
  radio.openReadingPipe(1,pipes[1]);
  radio.stopListening();

  radio.write(&myHeader, sizeof(myHeader));
  radio.write(&send_payload, sizeof(send_payload));
  Serial.print(myHeader.from_address, HEX);
  Serial.print(myName);
  Serial.print(": ");
  Serial.println(send_payload);
}

// ***Receive Message***
void receiveMessage() 
{
  // wait until there is a message to receive
  while (!radio.available()) 
  {
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
    
    RadioHeader receiveHeader={0, 0};
    uint8_t everything[100];

    radio.read(&everything, sizeof(everything));
    receiveHeader={everything[0], everything[1]};
    
    Serial.print(receiveHeader.to_address, HEX);
    Serial.print(receiveHeader.from_address, HEX);
    Serial.print("\n"); 

    // Find Curly Braces
    int i_first=0;
    int i_second=0;
    for(int i=0; i<100; i++){
      if(everything[i]==123)
        i_first=i;
      else if(everything[i]==125)
        i_second=i;
    }
    //Print the message between the curly braces
    for(int i=(i_first+1); i<(i_second); i++){
      printf("%c", everything[i]);
    }
}

// ***Write to EEPROM***
int eepromWrite(int startAddress, String string) 
{
  int strLength = string.length() + 1;
  char charBuff[strLength];
  string.toCharArray(charBuff, strLength);
  for(int i = startAddress; i <= strLength; i++) 
  {
    EEPROM.write(i, charBuff[i]);
  }
  EEPROM.write(nameLengthAddr, strLength);
  return strLength;
}

// ***Read from EEPROM***
String eepromRead(int startAddress, int strLength) 
{
  char charBuff[strLength];
  for(int i = startAddress; i <= strLength; i++) 
  {
    charBuff[i] = EEPROM.read(i);
  }
  return charBuff;  
}

// ***EEPROM Clear***
void eepromClear(int startAddress, int stopAddress) 
{
  for (int i = startAddress; i <= stopAddress; i++) 
  {
     EEPROM.write(i, 0);
  }
}

// ***Choose Power***
void choosePower(int POWER)  //Select which power level to run the nRF24L01+ at
{
    if(POWER == 0) 
    {
      radio.setPALevel(RF24_PA_MIN);
    }else if(POWER == 1) {
      radio.setPALevel(RF24_PA_LOW);
    }else if(POWER == 2) {
      radio.setPALevel(RF24_PA_HIGH);
    }else if(POWER == 3) {
      radio.setPALevel(RF24_PA_MAX);
    }else {
      radio.setPALevel(RF24_PA_MAX);
    }
}

// ***Set Data Rate***
void setRate(int RATE) //Select data rate to jam specific communication protocols
{ 
  if (RATE == 0) 
  {
    radio.setDataRate(RF24_250KBPS);
  }else if (RATE == 1) {
    radio.setDataRate(RF24_1MBPS);
  }else if (RATE == 2) {
    radio.setDataRate(RF24_2MBPS);
  }else {
    radio.setDataRate(RF24_1MBPS);
  }
}

// ***Set Channel Frequency***
void setChannel(int CHANNEL) 
{
  if (CHANNEL <= 127) 
  {
    radio.setChannel(CHANNEL);  
  }else {
    radio.setChannel(76);
  }
}

// ***Get My Name to be Globally available***
String getMyName(char myName[])
{
  String x = String(myName);
  return x;
}

