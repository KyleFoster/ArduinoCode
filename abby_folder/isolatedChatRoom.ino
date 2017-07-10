//Isolated Chat Room function

//Libraries
#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"
#include "printf.h"
#include "EEPROM.h"
#include <string.h>

RF24 radio(9,10);

#define my_node_index 0 //Change this to your respective address index 

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
  // put your setup code here, to run once:
  Serial.begin(9600);
  printf_begin();
  radio.begin();
  radio.setRetries(15,15);

  //Additional Setup up code that ultimately should be put in Carlos's Setup Code
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

void loop() {
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
  memset(send_payload, 0, 20);
  RadioHeader myHeader;

//  Serial.readBytesUntil(';',to_node,7);  
  Serial.readBytesUntil('\n',send_payload,20);
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
  Serial.print(myAddresses[my_node_index].userName);
  Serial.print(" -> ");
  Serial.print(myAddresses[to_node_index].userName);
  Serial.print(": ");
  Serial.println(send_payload);

  return messageSuccess;
}

