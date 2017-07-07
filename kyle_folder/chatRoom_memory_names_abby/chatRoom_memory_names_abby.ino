#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"
#include "printf.h"
#include "EEPROM.h"
#include <string.h>

RF24 radio(9,10);

char send_payload[100] = "";
char names[10];

int power, rate, channel;
int nameLength = 10;
int nameBegin =  0;
int nameLengthAddr = 11;
int powerAddr = 12;
int rateAddr = 13;
int channelAddr = 14;
String myName;

struct addressBook{
  String userName;
  uint8_t address;
} myAddresses[6]={{"Abby", 0xA1},{"Carlos", 0xB1},{"Kyle", 0xC1},{"Alex", 0xD1},{"Harman", 0xE1},{"Malik", 0xF1}}; 

struct RadioHeader {
  uint8_t to_address;
  uint8_t from_address;
};

void setup() {

  Serial.begin(9600);
  printf_begin();
  radio.begin();
  radio.enableDynamicPayloads();
  radio.setRetries(15, 15);
  radio.startListening();
  radio.setAutoAck(true);

  setupProfile();

  Serial.println("---------------Begin Chat!---------------\n\r");
}

void loop(void) 
{       
  receiveMessage();
}

// ***Send Message***
int sendMessage() 
{
  //Declare and Initialize Variables 
  int x=0;
  char to_node[7];
  memset(to_node, 0, 7);
  bool validAddress=false;
  memset(send_payload, 0, 100);
  RadioHeader myHeader;

  Serial.readBytesUntil(';',to_node,7);
  Serial.readBytesUntil('\n',send_payload,100);

  for(int i=0; i<6; i++){
    if(String(to_node)==myAddresses[i].userName){
      radio.openWritingPipe(myAddresses[i].address);
      myHeader={myAddresses[i].address, myAddresses[2].address};
      validAddress=true;
      x=i;
    }
  }
  if(!validAddress){
    Serial.println("Invalid transmit address.");
  }
  
  uint8_t totalMessage[102];
  memset(totalMessage, 0, 102);
  totalMessage[0]=myHeader.to_address;
  totalMessage[1]=myHeader.from_address;
  totalMessage[2]='{';

  for(int i=3; i<102; i++){
    totalMessage[i]=send_payload[i-3];
    if(send_payload[i-3]=='\0'){
      totalMessage[i]='}';
      i=103;
    }
  }
 
  radio.stopListening();
  radio.write(&totalMessage, sizeof(totalMessage));
  Serial.print(myName);
  Serial.print(" -> ");
  Serial.print(myAddresses[x].userName);
  Serial.print(": ");
  Serial.println(send_payload);

  return x;
}

// ***Receive Message***
void receiveMessage() 
{
  //Declare and Initialize Variables
  int x=0;
  
  // wait until there is a message to receive
  while (!radio.available()) 
  {
    if (Serial.available())
    {
      x=sendMessage();
      //Serial.println(x);
      radio.openWritingPipe(myAddresses[x].address);
     // radio.openReadingPipe(1,myAddresses[x].);
      radio.startListening();
    }  
  }
    
    RadioHeader receiveHeader={0, 0};
    uint8_t everything[102];
    memset(everything, 0, 102);

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

    //Find who the message was from
    int y=0;
    for(int i=0; i<6; i++){
      if(receiveHeader.from_address==myAddresses[i].address)
        y=i;
    }
    
    //Compare Addresses
    if(receiveHeader.to_address==myAddresses[2].address){
      //Print the message between the curly braces
      printf("%s: ", myAddresses[y].userName.c_str());
      for(int i=(i_first+1); i<(i_second); i++){
        printf("%c", everything[i]);
      }
      printf("\n");
    }
    else
      printf("This message is not for you, forwarding message");
    
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

// ***Setup***

void setupProfile()
{
  //nameLength = EEPROM.read(nameLengthAddr);
  //char names[nameLength];
  for(int i = nameBegin; i <= nameLength; i++) {
    names[i] = EEPROM.read(i);
  }
  myName = getMyName(names);
  Serial.println("Type 'Y' to load " + myName + "'s profile."); 
  Serial.println("Type 'YD' to load " + myName + "'s profile and print diagnostics.");
  Serial.println("Type any other key(s) to create a new profile.");
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
  else if (load == "YD")
  {
    power = EEPROM.read(powerAddr); //Recall saved parameters and skip setup dialogs
      choosePower(power);
    rate = EEPROM.read(rateAddr);
      setRate(rate);
    channel = EEPROM.read(channelAddr);
      setChannel(channel);
    radio.printDetails();
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

    myName = eepromRead(nameBegin, nameLength);
    radio.printDetails();
  }
  for(int i=1; i<7; i++){
    radio.openReadingPipe(i, myAddresses[i-1].address);
  }
 Serial.println("Address Book:\n\r");
  for(int i=0; i<6; i++){
     printf("%i) %s\n", i, myAddresses[i].userName.c_str());
  }
 Serial.println("\n\rType user name, semicolon, then message. eg: 'Kyle;You are garbage at chess.'");
}


