//Isolated Chat Room function

//Libraries
#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"
#include "printf.h"
#include "EEPROM.h"
#include <string.h>

RF24 radio(9,10);

#define my_node_index 5 //Change this to your respective address index 

//Structs
struct addressBook {
  String userName;
  uint8_t address;
} myAddresses[6]={{"Abby", 0xA1},{"Carlos", 0xB1},{"Kyle", 0xC1},{"Alex", 0xD1},{"Harman", 0xE1},{"Malik", 0xF1}}; 

struct RadioHeader {
  uint8_t to_address;
  uint8_t from_address;
  uint8_t final_address;
  char message_type;
};

uint8_t connectionTable[6] = {0,0,0,0,0,0};
bool has_five = false;
unsigned long previousMillis = 0; 
unsigned long interval = 2000;
int i = 0;


void setup() {
  Serial.begin(1000000);
  printf_begin();
  radio.begin();
  //radio.failureDetected = 0;
  radio.enableDynamicPayloads();
  radio.setRetries(5, 5);
  radio.setAutoAck(true);
  radio.setChannel(120);
  //radio.setDataRate(RF24_250KBPS);
  //Serial.println(radio.failureDetected);

   /*Open all writing pipes*/
  for (i = 1; i < 7; i++)
  { 
    radio.openReadingPipe(i, myAddresses[i-1].address);
  }
  radio.startListening();

  /*Print out contact list*/
  Serial.println(F("Address Book:\n\r"));
  for (i = 0; i < 6; i++)
     printf("%s\n", myAddresses[i].userName.c_str());
     
  Serial.println(F("\n\r- To Send a message:"));
  Serial.println(F("Type user name, semicolon, then message.\n\reg: 'Alex;You are equally as garbage at chess.'"));
  Serial.println(F("------------Begin Chat-------------"));

}

void loop() {
  receiveMessage();
}


void receiveMessage() 
{
  int final_node_index = 6;
  int to_node_index = 6;
  int from_node_index = 0;

  /*Hardcode content of the broadcast message*/
  uint8_t broadcastMessage[4];
  broadcastMessage[0] = myAddresses[my_node_index].address; //hardcode contents of broadcastMessage
  broadcastMessage[1] = myAddresses[my_node_index].address; 
  broadcastMessage[2] = myAddresses[my_node_index].address;
  broadcastMessage[3] = 'B';
  
  int start_time = millis();
  
  // wait until there is a message to receive
  while (!radio.available()) 
  {
    unsigned long currentMillis=millis();
    if (Serial.available())
    {
        to_node_index=readUserInput(); //Calls when the user inputs data to the serial monitor
        Serial.println(F("returned to receiveMessage"));
    }
    else
    { 
      if ((currentMillis - previousMillis > interval))
      {
        previousMillis = currentMillis;
        Serial.println(F("Broadcasting...\n"));
        radio.stopListening();
        //radio.flush_tx();
        radio.write(&broadcastMessage, sizeof(broadcastMessage)); //Broadcast message to update connections
        radio.startListening();
      }
    }
  }  
    
  RadioHeader receiveHeader = {0, 0, 0, '\0'};
  uint8_t everything[102];
  memset(everything, 0, 102);
  radio.read(&everything, sizeof(everything));
  receiveHeader = {everything[0], everything[1], everything[2], everything[3]};

  //Check that we are receiving messages, delete later
  Serial.println("Addresses in receive message");
  Serial.print(receiveHeader.to_address, HEX);
  Serial.print(receiveHeader.from_address, HEX);
  Serial.print(receiveHeader.final_address, HEX);
  Serial.print(receiveHeader.message_type);
  Serial.print("\n"); 

  //Find to_node_index
  for (i = 0; i<6; i++)
  {
    if (receiveHeader.to_address==myAddresses[i].address)
      to_node_index = i;
  }
  
  //Find from_node_index
  for (i = 0; i < 6; i++)
  {
    if (receiveHeader.from_address==myAddresses[i].address)
      from_node_index = i;
  }

  //Find final_node_index
  for (i=0; i<6; i++)
  {
    if (receiveHeader.final_address==myAddresses[i].address)
      final_node_index = i;
  }

  //Figure out what to do with the message
  int message_decision = 0;
  message_decision=messageDecide(receiveHeader, to_node_index, from_node_index, final_node_index); 
  Serial.println(F("returned to receiveMessage"));
  printf("message type: %i\n", message_decision);

  
  switch(message_decision){
    case 0: 
      printf("\n-----------------------Garbage-----------------------\n");
      radio.flush_rx();
      break;
    case 1:
      //Print the message
      printf("\n----------------------------------------");
      printf("%s: ", myAddresses[from_node_index].userName.c_str());
      for (int i = 4; i < 102; i++)
        printf("%c", everything[i]);
      printf("----------------------------------------\n");
      break;
     case 2:
      //Relay Message
      printf("Relaying Data");
      to_node_index = checkConnection(final_node_index, from_node_index);
      Serial.println(F("returned to receiveMessage"));
      everything[0] = myAddresses[to_node_index].address;
      Serial.println(everything[0], HEX);
      radio.stopListening();
      //radio.flush_tx();
      radio.write(&everything, sizeof(everything));
      radio.startListening();
      break;
     case 3:
      //Broadcast Message
      break;
    default: 
      //do nothing
      break;
  }  
}


int readUserInput(){
  Serial.println(F("Entered readUserInput"));
 
  char prefix[10];
  int final_node_index=6;
  int to_node_index=6;

  memset(prefix, 0, 10);

  /*Read in user input until ;*/
  Serial.readBytesUntil(';',prefix,10); 
  
  /*Decide what user wants to do and call appropriate functions*/
  if (String(prefix) == "ChangeCH"||String(prefix) == "ChangePA"||String(prefix) == "ChangeRT")
  {
    /*Do nothing*/
    final_node_index=6;
  }
  else if (String(prefix) == "PrintCT")
  {
    for (i=0; i<6; i++)
      printf("%i ", connectionTable[i]);
    printf("\n");
  }
  else 
  {
    for (i=0; i<6; i++)
    {
      if (String(prefix) == myAddresses[i].userName){
        final_node_index = i;
        to_node_index = checkConnection(final_node_index, my_node_index);
        Serial.println(F("returned to readUserInput"));
        i = 7;
      }
    }
    if(to_node_index<6)
    {
       sendMessage(final_node_index, to_node_index);
       Serial.println(F("returned to readUserInput"));
    }
  }
  return final_node_index;
}


int sendMessage(int final_node_index, int to_node_index){
  //Declare and Initialize Variables 
  bool messageSuccess=false;
  char send_payload[100] = "";
  memset(send_payload, 0, 100);
  RadioHeader myHeader;

  Serial.readBytesUntil('\n',send_payload,100);

  radio.openWritingPipe(myAddresses[to_node_index].address);
  myHeader={myAddresses[to_node_index].address, myAddresses[my_node_index].address, myAddresses[final_node_index].address, 'M'};
//  Serial.println("Addresses in myHeader");
//  Serial.println(myHeader.to_address, HEX);
//  Serial.println(myHeader.from_address, HEX);
//  Serial.println(myHeader.final_address, HEX);
  
  uint8_t totalMessage[102];
  memset(totalMessage, 0, 102);
  totalMessage[0] = myHeader.to_address;
  totalMessage[1] = myHeader.from_address;
  totalMessage[2] = myHeader.final_address;
  totalMessage[3] = myHeader.message_type;

  for(int i = 4; i < 102; i++){
    totalMessage[i] = send_payload[i - 4];
  }
 
  radio.stopListening();
  //radio.flush_tx();
  messageSuccess = radio.write(&totalMessage, sizeof(totalMessage));
  Serial.print(myAddresses[my_node_index].userName);
  Serial.print(" -> ");
  Serial.print(myAddresses[final_node_index].userName);
  Serial.print(": ");
  Serial.println(send_payload);
  radio.startListening();

  return messageSuccess;
}

void updateTable(int table_index){
  Serial.println(F("Entered updateTable"));
  int buffer_position=connectionTable[table_index];
  if (buffer_position!=1)
  {
    if (!has_five && buffer_position == 0)
    {
      Serial.println("top");
      for (i=0; i<6; i++)
      {
        if (connectionTable[i]!=0)
        {
          connectionTable[i]++;
          if (connectionTable[i]==5)
            has_five=true;
        }
      }
    }
    else
    {
      Serial.println("bottom");
      for(i=0; i<6; i++)
      {
        if(connectionTable[i]<buffer_position && connectionTable[i] != 0)
          connectionTable[i]++;
      }
    }
    connectionTable[table_index]=1;
  }
}

int messageDecide(RadioHeader &receiveHeader, int to_node_index, int from_node_index, int final_node_index){
  int messageDecision=0; // Return variable
  
  if (receiveHeader.to_address == myAddresses[my_node_index].address)
  {
    if (receiveHeader.final_address==myAddresses[my_node_index].address && receiveHeader.message_type=='M') //Message for you to read
      messageDecision=1;
    else if (receiveHeader.final_address!=myAddresses[my_node_index].address && receiveHeader.message_type=='M') //Message you need to relay
      messageDecision=2;
  }
  else 
  {
    if (receiveHeader.message_type=='B'&& receiveHeader.from_address!=myAddresses[my_node_index].address)
    { 
      updateTable(from_node_index);
      messageDecision=3;
    }
  }  
  return messageDecision;  
}

int checkConnection(int final_node_index, int from_node_index){
  Serial.println(F("entered checkConnection"));
  int to_node_index = 6;
  int checkValue = 1;
  if (connectionTable[from_node_index]==checkValue)
    checkValue++;
  if (connectionTable[final_node_index]!=0)
    to_node_index=final_node_index;
  else
  {
    for (i=0; i<6; i++){
      if (connectionTable[i]==checkValue)
        to_node_index=i;
    }
    if (to_node_index==6)
      Serial.println(F("You are not connected to the mesh."));
  }
  printf("to_node_index: %i\n", to_node_index);
  return to_node_index;
}
