// Updated "Broadcast Method" for full mesh network functionality.. this will work.

//Libraries
#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"
#include "printf.h"
#include "EEPROM.h"
#include <string.h>

RF24 radio(9,10);

#define my_node_index 1 //Change this to your respective address index 

//Structs
struct addressBook{
  String userName;
  uint8_t address;
} myAddresses[6] = {{"Abby", 0xA1},{"Carlos", 0xB1},{"Kyle", 0xC1},{"Alex", 0xD1},{"Harman", 0xE1},{"Malik", 0xF1}}; 

struct RadioHeader {
  uint8_t to_address;
  uint8_t from_address;
  uint8_t final_address;
  char message_type;
};

RadioHeader receiveHeader={0, 0, 0, '\0'};
  uint8_t everything[102];
  memset(everything, 0, 102);
  radio.read(&everything, sizeof(everything));
  receiveHeader={everything[0], everything[1], everything[2], everything[3]};

uint8_t connectionTable[6] = {0,0,0,0,0,0};
bool has_five = false;
unsigned long previousMillis = 0; 
unsigned long interval = 5000; // 5 second delay

//Function Prototypes
  //void setup()
  //void loop()
  //int sendMessage(int to_node_index)
  //void receiveMessage()
  //int readUserInput
  //int checkConnection(int final_node_index, int from_node_index)

void setup()
{
  Serial.begin(38400);
  printf_begin();
  radio.begin();
  radio.enableDynamicPayloads();

  // Open all of the reading pipes
  int j = 0;
  for (int i = 1; i < 6; i++)
  {
    if (j = my_node_index) 
    {
      j++;
    }
    radio.openReadingPipe(i, myAddresses[j].address);
    j++;
  }
  radio.startListening();
  
  // Print out contact list
  Serial.println(F("Address Book:\n\r"));
  for (int i = 0; i < 6; i++)
  {
    printf("%s\n", myAddresses[i].userName.c_str());
  }
  
  Serial.println(F("\n\r- To Send a message:"));
  Serial.println(F("Type user name, semicolon, then message.\n\reg: 'Alex;You are equally as garbage at chess.'"));
  Serial.println(F("------------Begin Chat-------------"));
}

void loop() 
{
  while (!radio.available()) // Wait for incoming data/message
  {
    unsigned long currentMillis = millis(); // Start counting for broadcast timer
    if (Serial.available()) // Checks for user input from Serial Monitor
    {
      sendMessage(); 
    } 
    else 
    { 
      if ((currentMillis - previousMillis > interval))
      {
        previousMillis = currentMillis;
        broadcastMessage();
      }
    }
  receiveMessage();
}








// ***Broadcast Message***
void broadcastMessage(void) 
{
  uint8_t broadcastMessage[4];
  
  broadcastMessage[0] = myAddresses[my_node_index].address; // Hardcode contents of broadcastMessage
  broadcastMessage[1] = myAddresses[my_node_index].address; 
  broadcastMessage[2] = myAddresses[my_node_index].address;
  broadcastMessage[3] = 'B';
  
  Serial.println(F("Broadcasting...\n"));
  radio.stopListening();
  radio.write(&broadcastMessage, sizeof(broadcastMessage)); //Broadcast message to update connections
  radio.startListening();
}





 
