// Change indices on lines 73 and 155 to respective numbers.

//Libraries
#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"
#include "printf.h"
#include "EEPROM.h"
#include <string.h>

RF24 radio(9,10);

#define my_node_index 0

//Global Variables
char names[10];

int power, rate, channel;
int nameLength=10;
int nameBegin=0;
int nameLengthAddr=11;
int powerAddr=12;
int rateAddr=13;
int channelAddr=14;
String myName;

//Structs
struct addressBook{
  String userName;
  uint8_t address;
} myAddresses[6]={{"Abby", 0xA1},{"Carlos", 0xB1},{"Kyle", 0xC1},{"Alex", 0xD1},{"Harman", 0xE1},{"Malik", 0xF1}}; 

struct RadioHeader {
  uint8_t to_address;
  uint8_t from_address;
};

//Function Prototypes
  //void setup()
  //void loop()
  //int sendMessage(int to_node_index)
  //void receiveMessage()
  //int eepromWrite(int startAddress, String string) 
  //String eepromRead(int startAddress, int strLength)
  //void eepromClear(int startAddress, int stopAddress) 
  //void setRate(int RATE)
  //int setChannel(int CHANNEL) 
  //String getMyName(char myName[])
  //void setupProfile()
  //void updateParameters(String input) 
  
void setup() {

  Serial.begin(9600);
  printf_begin();
  radio.begin();
  radio.enableDynamicPayloads();
  radio.setRetries(15, 15);
  radio.startListening();
  radio.setAutoAck(true);

  setupProfile();

  Serial.println("----------------------------------------Begin Chat!---------------------------------------\n\r");
}

void loop(void) 
{       
  receiveMessage();
}

int readUserInput(){
  //Declare and Initialize Variables
  int to_node_index;
  char prefix[7];
  memset(prefix, 0, 7);

  //Get Serial input until ;
  Serial.readBytesUntil(';',prefix,7); 
  
  //Decide what user wants to do and call appropriate functions
  if(String(prefix)=="ChangeCH"||String(prefix)=="ChangePA"||String(prefix)=="ChangeRT"){
    //Carlos put change code here
    updateParameters(prefix); 
    to_node_index=6;
  }
  else{
    for(int i=0; i<6; i++){
      if(String(prefix)==myAddresses[i].userName){
        to_node_index=i;
        sendMessage(i);
      }
    }
  }
  return to_node_index;
}

// ***Receive Message***
void receiveMessage() 
{
  //Declare and Initialize Variables
  int x;
  
  // wait until there is a message to receive
  while (!radio.available()) 
  {
    if (Serial.available())
    {
        x=readUserInput();
        if(x<6){
          radio.openWritingPipe(myAddresses[x].address);
          radio.startListening();
        }
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
    int i_first = 0;
    int i_second = 0;
    for(int i = 0; i < 100; i++){
      if(everything[i] == 123)
        i_first = i;
      else if(everything[i] == 125)
        i_second = i;
    }

    //Find who the message was from
    int y = 0;
    for(int i = 0; i < 6; i++){
      if(receiveHeader.from_address==myAddresses[i].address)
        y = i;
    }
    
    //Compare Addresses
    if(receiveHeader.to_address == myAddresses[0].address){
      //Print the message between the curly braces
      printf("%s: ", myAddresses[y].userName.c_str());
      for(int i = (i_first+1); i < (i_second); i++){
        printf("%c", everything[i]);
      }
      printf("\n");
    }
    else {
      Serial.println(F("Forwarding Message"));
    }    
}

// ***Send Message***
int sendMessage(int to_node_index) 
{
  //Declare and Initialize Variables 
//  int x = 0;
//  char to_node[7];
 // memset(to_node, 0, 7);
  bool messageSuccess=false;
  char send_payload[20] = "";
  memset(send_payload, 0, 100);
  RadioHeader myHeader;

//  Serial.readBytesUntil(';',to_node,7);  
  Serial.readBytesUntil('\n',send_payload,100);
  Serial.println(send_payload);

  //Open Writing Pipe
  radio.openWritingPipe(myAddresses[to_node_index].address);
  myHeader={myAddresses[to_node_index].address, myAddresses[my_node_index].address};

//  for(int i = 0; i < 6; i++)
//  {
//    if (String(to_node) == myAddresses[i].userName)
//    {
//        radio.openWritingPipe(myAddresses[i].address);
//        myHeader={myAddresses[i].address, myAddresses[1].address};
//        validAddress = true;
//        x = i;
//    }
//  }
//  
//  if(!validAddress)
//  {
//    Serial.println("Invalid transmit address.\n\r");
//  }
  
  uint8_t totalMessage[102];
  memset(totalMessage, 0, 102);
  totalMessage[0] = myHeader.to_address;
  totalMessage[1] = myHeader.from_address;
  totalMessage[2] = '{';

  for(int i = 3; i < 102; i++){
    totalMessage[i] = send_payload[i - 3];
    if(send_payload[i - 3] == '\0')
    {
      totalMessage[i] = '}';
      i = 103;
    }
  }
 
  radio.stopListening();
  messageSuccess=radio.write(&totalMessage, sizeof(totalMessage));
  Serial.print(myName);
  Serial.print(" -> ");
  Serial.print(myAddresses[to_node_index].userName);
  Serial.print(": ");
  Serial.println(send_payload);

  return messageSuccess;
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
      Serial.println("***Invalid Power Level. Setting to Default: 0dBm***\n\r");
      radio.setPALevel(RF24_PA_MAX);
    }
}

// ***Set Data Rate***
void setRate(int RATE) //Select data rate to jam specific communication protocols
{ 
  if (RATE == 0) 
  {
    radio.setDataRate(RF24_250KBPS);
    EEPROM.write(rateAddr, RATE);
  }else if (RATE == 1) {
    radio.setDataRate(RF24_1MBPS);
    EEPROM.write(rateAddr, RATE);
  }else if (RATE == 2) {
    radio.setDataRate(RF24_2MBPS);
    EEPROM.write(rateAddr, RATE);
  }else {
    Serial.println(F("***Invalid Data Rate. Setting to Default: 1MBPS***\n\r"));
    radio.setDataRate(RF24_1MBPS);
    EEPROM.write(rateAddr, 1);
  }
}

// ***Set Channel Frequency***
int setChannel(int CHANNEL) 
{
  if (CHANNEL <= 127) 
  {
    radio.setChannel(CHANNEL); 
    EEPROM.write(channelAddr, CHANNEL); 
    return CHANNEL;
  }else {
    Serial.println(F("***Invalid Channel. Setting to Default: 76***\n\r"));
    radio.setChannel(76);
    EEPROM.write(channelAddr, 76);
    return 76;
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
  for(int i = nameBegin; i <= nameLength; i++) {
    names[i] = EEPROM.read(i);
  }
  myName = getMyName(names);
  delay(500);
  Serial.println("***********************************************************************");
  Serial.println(F("*--------------------Welcome to the Mesh Chatroom---------------------*"));
  Serial.println("* - Type 'Y' to load " + myName + "'s profile.                                *"); 
  Serial.println("* - Type 'YD' to load " + myName + "'s profile and print diagnostics.         *");
  Serial.println(F("* - Type any other key(s) to create a new profile.                    *"));
  Serial.println(F("***********************************************************************"));
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
    power = EEPROM.read(powerAddr); //Recall saved parameters, print debugging info and skip setup dialogs
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

    Serial.println(F("Choose Data Rate:\n\r 0: 250kbps\n\r 1: 1Mbps\n\r 2: 2Mbps\n\r"));
    Serial.flush();
    while(!Serial.available());
    rate = Serial.parseInt();
    setRate(rate); 

    Serial.println(F("Set Channel: Enter 0 - 127 or 76 for default"));
    Serial.flush();
    while(!Serial.available());
    channel = Serial.parseInt();
    float frequency = ((2400 + channel) * 0.001);
    int actualChannel = setChannel(channel);
    Serial.print("Channel set to ");
    Serial.print(actualChannel);
    Serial.print(". Frequency = ");
    Serial.print((float)(frequency),3);
    Serial.println(F(" GHz\n\r"));
    
    myName = eepromRead(nameBegin, nameLength);
    radio.printDetails();
  }
  for(int i = 1; i < 7; i++){ // Open all writing pipes
    radio.openReadingPipe(i, myAddresses[i - 1].address);
  }
  Serial.println(F("Address Book:\n\r")); // Print out contact list
  for(int i = 0; i < 6; i++){
     printf("%s\n", myAddresses[i].userName.c_str());
  }
  Serial.println(F("\n\r- To Send a message:"));
  Serial.println(F("Type user name, semicolon, then message.\n\reg: 'Alex;You are equally as garbage at chess.'"));
}

// ***Update Parameters***
void updateParameters(String input) 
{
  if (input == "ChangeCH") 
  {
    Serial.println(F("Enter New Channel 0 - 127: "));
    Serial.flush();
    while(!Serial.available());
    int val = Serial.parseInt();
    if (val <= 127) 
    {
      setChannel(val);
      printf("%i\n\r", val);
    } else 
    {
      setChannel(76);
      Serial.println(F("Invalid Channel. Setting to Default: 76"));
    }
  }else if (input == "ChangePA") 
  {
    Serial.println(F("Enter New TX Power Level:\n\r 0: -18dBm\n\r 1: -12dBm\n\r 2: -6dBm \n\r 3: 0dBm  \n\r"));
    Serial.flush();
    while(!Serial.available());
    int val = Serial.parseInt();
    if (val < 4) 
    {
      choosePower(val);
      printf("New TX Power Level: %i\n\r", val);
    }else 
    {
      choosePower(3);
      printf("Invalid Power Level. Setting to Default: 0dBm");
    }
  }else if (input == "ChangeRT") 
  {
    Serial.println(F("Enter New Data Rate:\n\r 0: 250kbps\n\r 1: 1Mbps\n\r 2: 2Mbps\n\r"));
    Serial.flush();
    while(!Serial.available());
    int val = Serial.parseInt();
    if (val < 3) 
    {
      setRate(val);
      printf("New Data Rate: %i\n\r", val);
    }else 
    {
      setRate(1);
      printf("Invalid Data Rate. Setting to Default: 1Mbps");
    }
   }else {
    Serial.println(F("Invalid Entry/Syntax."));
    Serial.println(F("To Send a Message, Use the Form: 'Alex;You are equally as garbage at chess'"));
    Serial.println(F("Or to Change a Parameter, Type: 'ChangeCH', 'ChangePA', or ChangeRT for Channel, TX Power, or Data Rate respectively."));
   }
}

