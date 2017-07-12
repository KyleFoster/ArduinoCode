//Isolated Chat Room function

//Libraries
#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"
#include "printf.h"
#include "EEPROM.h"
#include <string.h>

RF24 radio(9,10);

#define my_node_index 2 //Change this to your respective address index 

//Structs
struct addressBook{
  String userName;
  uint8_t address;
} myAddresses[6]={{"Abby", 0xA1},{"Carlos", 0xB1},{"Kyle", 0xC1},{"Alex", 0xD1},{"Harman", 0xE1},{"Malik", 0xF1}}; 

struct RadioHeader {
  uint8_t to_address;
  uint8_t from_address;
  char message_type;
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
  Serial.begin(1000000);
  printf_begin();
  radio.begin();
  radio.enableDynamicPayloads();
  radio.setRetries(15, 15);
  radio.setAutoAck(true);
  radio.setChannel(70);

  //Additional Setup up code that ultimately should be put in Carlos's Setup Code
  for(int i = 1; i < 7; i++){ // Open all writing pipes
    radio.openReadingPipe(i, myAddresses[i - 1].address);
  }
  radio.startListening();

  Serial.println(F("Address Book:\n\r")); // Print out contact list
  for(int i = 0; i < 6; i++){
     printf("%s\n", myAddresses[i].userName.c_str());
  }
  Serial.println(F("\n\r- To Send a message:"));
  Serial.println(F("Type user name, semicolon, then message.\n\reg: 'Alex;You are equally as garbage at chess.'"));
  Serial.println(F("------------Begin Chat-------------"));

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
        //printf("returned to readUserInput\n");
        i=7;
      }
    }
  }
  return to_node_index;
}

// ***Receive Message***
void receiveMessage() 
{
//  printf("entered receiveMessage function\n");
  //Declare and Initialize Variables
  int x;
  
  // wait until there is a message to receive
  while (!radio.available()) 
  {
    if (Serial.available())
    {
        x=readUserInput();
        int endTime=millis();
        printf("%i",endTime);
        //printf("returned to receiveMessage\n");
        if(x<6){
          radio.openWritingPipe(myAddresses[x].address);
          radio.startListening();
        }
    }  
   }  
    
    RadioHeader receiveHeader={0, 0, '\0'};
    uint8_t everything[102];
    memset(everything, 0, 102);
    radio.read(&everything, sizeof(everything));
    receiveHeader={everything[0], everything[1], everything[2]};
    //printf("%c", receiveHeader.message_type);

            for(int i = 3; i < 102; i++){
          printf("%c", everything[i]);
        }
    
    //Serial.print(receiveHeader.to_address, HEX);
    //Serial.print(receiveHeader.from_address, HEX);
    //Serial.print("\n"); 

    //Find who the message is to
    for(int i=0; i<6; i++){
      if(receiveHeader.to_address==myAddresses[i].address)
        x = i;
    }
    
    //Find who the message was from
    int y = 0;
    for(int i = 0; i < 6; i++){
      if(receiveHeader.from_address==myAddresses[i].address)
        y = i;
    }
    
    //Compare Addresses
    if(receiveHeader.to_address == myAddresses[my_node_index].address){
      if(receiveHeader.message_type=='M'){
        //Print the message
        printf("%s: ", myAddresses[y].userName.c_str());
        for(int i = 3; i < 102; i++){
          printf("%c", everything[i]);
        }
        printf("\n");
        //Return a success message
        uint8_t returnMessage[3];
        returnMessage[0]=myAddresses[y].address;
        returnMessage[1]=myAddresses[my_node_index].address;
        returnMessage[2]='A';
        radio.openWritingPipe(myAddresses[y].address);
        radio.stopListening(); 
        /*
        while(startTime+150>endTime){
          startTime=
        }
        */
        radio.write(&returnMessage, sizeof(returnMessage));
        radio.startListening();
      }
      else if(receiveHeader.message_type=='A'){
        printf("%s has received your message\n", myAddresses[y].userName.c_str());
      }
      else if(receiveHeader.message_type=='B'){
        //Discard Broadcast Message
      }
    }
    else {
      if (receiveHeader.from_address!=myAddresses[my_node_index].address){
        radio.openWritingPipe(myAddresses[x].address);
        radio.stopListening();
        radio.write(&everything, sizeof(everything));
        radio.startListening();
        /* Testing routing
        Serial.println("Enter else");
        uint8_t testMessage[5];
        memset(testMessage, 0, 5);
        testMessage[0]=myAddresses[2].address; 
        testMessage[1]=myAddresses[0].address;
        testMessage[2]='M';
        testMessage[3]='!';
        radio.stopListening();
        radio.openWritingPipe(myAddresses[x].address);
        radio.write(&testMessage, sizeof(testMessage));
        radio.startListening();
        */
      }
    }    
}

// ***Send Message***
int sendMessage(int to_node_index) 
{
//  printf("entered sendMessage function\n");
  //Declare and Initialize Variables 
  bool messageSuccess=false;
  char send_payload[100] = "";
  memset(send_payload, 0, 100);
  RadioHeader myHeader;

//  Serial.readBytesUntil(';',to_node,7);  
  Serial.readBytesUntil('\n',send_payload,100);
//  Serial.println(send_payload);

  //Open Writing Pipe
  radio.openWritingPipe(myAddresses[to_node_index].address);
  myHeader={myAddresses[to_node_index].address, myAddresses[my_node_index].address, 'M'};
  
  uint8_t totalMessage[102];
  memset(totalMessage, 0, 102);
  totalMessage[0] = myHeader.to_address;
  totalMessage[1] = myHeader.from_address;
  totalMessage[2] = myHeader.message_type;
// Serial.println(myHeader.message_type);

  for(int i = 3; i < 102; i++){
    totalMessage[i] = send_payload[i - 3];
  }
 
  radio.stopListening();
  messageSuccess=radio.write(&totalMessage, sizeof(totalMessage));
  int startTime=millis();
  printf("%i", startTime);
  Serial.print(myAddresses[my_node_index].userName);
  Serial.print(" -> ");
  Serial.print(myAddresses[to_node_index].userName);
  Serial.print(": ");
  Serial.println(send_payload);
  radio.startListening();

  return messageSuccess;
}


