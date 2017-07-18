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
  uint8_t ttl;
  char message_type;
};

struct ConnectionTable
{ 
  uint8_t value;
  uint8_t updates_til_reset;
} c_t[6] = {{0,10}, {0,10}, {0,10}, {0,10}, {0,10}, {0,10}}; 

//ConnectionTable c_t[6] = {{0,10}, {0,10}, {0,10}, {0,10}, {0,10}, {0,10}};
//for (int i = 0; i < 6; i++)
//{
//  c_t[i].value = 0;
//  c_t[i].updates_til_reset = 10;
//}

RadioHeader receiveHeader = {0, 0, 0, 5, '\0'};

uint8_t received_message[32];
//uint8_t connectionTable[6] = {0,0,0,0,0,0};
uint8_t channel;
bool has_five = false;
unsigned long previousMillis = 0; 
unsigned long interval = 5000; // 5 second delay
char user_input[5] = "";

//Function Prototypes
  //void setup()
  //void loop()
  //void sendMessage(RadioHeader &sendHeader)
  //void receiveMessage()
  //RadioHeader readUserInput()
  //int checkConnection(int final_node_index, int from_node_index)

void setup()
{
  Serial.begin(38400);
  printf_begin();
  radio.begin();
  radio.enableDynamicPayloads();
  radio.setAutoAck(true);

  /* Open all the reading pipes */
  for (int i = 1; i < 7; i++)
  {
    radio.openReadingPipe(i, myAddresses[i - 1].address);
  }  
  radio.startListening();

  /*Scan or set to random channel between 0-127*/
  Serial.println();
  Serial.println("Enter 1 to set your own channel to a random channel between 0-127, enter anything else to scan: ");

  while (!Serial.available()) { }
  Serial.readBytesUntil('\n',user_input,5);
  String user_string = String(user_input);

  if (user_string == "1")
  {
    channel = random(0,127);
    printf("Channel set to: %d\n\r", channel);
    radio.setChannel(channel);
  }
  else
  {
    scanChannels();
  }
  /*******************************************/
  
  /* Print out contact list */
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
  RadioHeader sendHeader;
  while (!radio.available()) // Wait for incoming data/message
  {
    sendHeader = {0, 0, 0, 5, '\0'};
    unsigned long currentMillis = millis(); // Start counting for broadcast timer
    if (Serial.available()) // Checks for user input from Serial Monitor
    {
      sendHeader = readUserInput();
      if (sendHeader.to_address != myAddresses[my_node_index].address) 
      {
        sendMessage(sendHeader);
      }
    } 
    else 
    { 
      if ((currentMillis - previousMillis > interval))
      {
        previousMillis = currentMillis;
        broadcastMessage();
      }
    }
  }
    receiveMessage();
}

// ***Receive Message ***
void receiveMessage(void)
{
  Serial.println(F("In receiveMessage"));
  //Declare and Initialize Variables
  int final_node_index = 6;
  int to_node_index = 6;
  int from_node_index = 6;
  RadioHeader receive_header={0, 0, 0, 5, '\0'};

  memset(received_message, 0, 32);
  radio.read(&received_message, sizeof(received_message));
  receive_header = {received_message[0], received_message[1], received_message[2], received_message[3], received_message[4]};  

  //Check that we are receiving messages, delete later
  Serial.println("Addresses in receive message");
  Serial.print(receive_header.to_address, HEX);
  Serial.print(receive_header.from_address, HEX);
  Serial.print(receive_header.final_address, HEX);
  Serial.print(receive_header.ttl);
  Serial.print(receive_header.message_type);
  Serial.print("\n");

  //Find indicies
  for (int i = 0; i < 6; i++)
  {
    if (receive_header.to_address == myAddresses[i].address)
      to_node_index = i;
    if (receive_header.from_address == myAddresses[i].address)
      from_node_index = i;
    if (receive_header.final_address == myAddresses[i].address)
      final_node_index = i;
  }
  
    messageDecide(receive_header, to_node_index, from_node_index, final_node_index);   
}

// ***Message Decide***
void messageDecide(RadioHeader &decide_header, int to_node_index, int from_node_index, int final_node_index)
{
  Serial.println(F("Inside messageDecide"));
  //Check for garbage first
  if (decide_header.message_type != 'M' && decide_header.message_type != 'B')
  {
    Serial.println("/////////GARBAGE ALERT!!!////////////");
    radio.begin();
    for (int i = 1; i < 7; i++)
    {
      radio.openReadingPipe(i, myAddresses[i - 1].address);
    }  
    radio.startListening();
  }
  else if (decide_header.to_address == myAddresses[my_node_index].address)
  {
    if (decide_header.final_address == myAddresses[my_node_index].address && decide_header.message_type == 'M') //Message for you to read
      {
        printf("\n----------------------------------------");
        printf("%s: ", myAddresses[from_node_index].userName.c_str());
        for (int i = 5; i < 32; i++)
        {
          printf("%c", received_message[i]);
        }
        printf("----------------------------------------\n");
      }
    else if (decide_header.final_address != myAddresses[my_node_index].address && decide_header.message_type == 'M') //Message you need to relay
      {
        if (decide_header.ttl > 0)
          relayMessage(final_node_index, from_node_index);
      }
  }
  else 
  {
    if (decide_header.message_type=='B'&& decide_header.from_address != myAddresses[my_node_index].address) //Broadcast Message, update the connection table
    { 
      updateTable(from_node_index);
    }
  }
}

// ***Relay Message***
void relayMessage(int final_node_index, int from_node_index) 
{
  Serial.println(F("Inside relayMessage"));
  int to_node_index;
  to_node_index = checkConnection(final_node_index, from_node_index);
  received_message[0] = myAddresses[to_node_index].address;
  received_message[3]--;
  radio.stopListening();
  radio.openWritingPipe(myAddresses[to_node_index].address);
  radio.write(&received_message, sizeof(received_message));
  radio.startListening();
}

// ***Update the Stack***
void updateTable(int table_index)
{
  Serial.println(F("Inside updateTable"));
  int buffer_position = c_t[table_index].value;
  if (buffer_position != 1)
  {
    if (!has_five && buffer_position == 0)
    {
      for (int i = 0; i < 6; i++)
      {
        if (c_t[i].value != 0)
        {
          c_t[i].value++;
          if (c_t[i].value == 5)
            has_five = true;
        }
      }
    }
    else
    {
      for (int i = 0; i < 6; i++)
      {
        if (c_t[i].value < buffer_position && c_t[i].value != 0)
          c_t[i].value++;
      }
    }
    c_t[table_index].value = 1;
  }
  for (int i = 0; i < 6; i++)  //update update_til_reset_values
  {
    if (c_t[i].value == 1)     //reset new number 1 to update_til_reset = 10
      c_t[i].updates_til_reset = 10;
    else                       //else minus 1 for all other update_til_resets 
    {
      if (c_t[i].value != 0)   //minus one for all except when c_t.values == 0
        c_t[i].updates_til_reset--;
      if (c_t[i].updates_til_reset == 0)  //if update_til_reset == 0 then reset value in stack to 0, ie unconnected
      {
        c_t[i].value = 0;
        c_t[i].updates_til_reset = 10;
      }
    }
  }
  /* Print c_t */
  for (int i=0; i<6; i++)
  {
    printf("%i ", c_t[i].value);
  }
    printf("\n");
  for (int i = 0; i < 6; i++) 
  {
    printf("%i ", c_t[i].updates_til_reset);
  }
  Serial.println();
}

// ***Send Message***
void sendMessage(RadioHeader &sendHeader) 
{
  Serial.println(F("Inside sendMessage"));
  char send_payload[32] = "";
  memset(send_payload, 0, 32);
  int final_node_index;

  Serial.readBytesUntil('\n',send_payload,32);
  Serial.println(send_payload);

  uint8_t totalMessage[32];
  memset(totalMessage, 0, 32);
  totalMessage[0] = sendHeader.to_address;
  totalMessage[1] = sendHeader.from_address;
  totalMessage[2] = sendHeader.final_address;
  totalMessage[3] = sendHeader.ttl;
  totalMessage[4] = sendHeader.message_type;

  for (int i = 5; i < 32; i++)
  {
    totalMessage[i] = send_payload[i - 5];
  }

  for (int i = 0; i < 6; i++)
  {
    if (sendHeader.final_address == myAddresses[i].address)
    {
      final_node_index = i;
    }
  }
  
  radio.stopListening();
  radio.openWritingPipe(sendHeader.to_address);
  radio.write(&totalMessage, sizeof(totalMessage));
  Serial.print(myAddresses[my_node_index].userName);
  Serial.print(" -> ");
  Serial.print(myAddresses[final_node_index].userName);
  Serial.print(": ");
  Serial.println(send_payload);
  radio.startListening();
}

// ***Read User Input***
RadioHeader readUserInput(void)
{
  Serial.println(F("Inside readUserInput"));
  //Declare and Initialize Variables
  char prefix[10];
  memset(prefix, 0, 10);
  int final_node_index = 6;
  int to_node_index = 6;
  RadioHeader returnHeader = {myAddresses[my_node_index].address, myAddresses[my_node_index].address, myAddresses[my_node_index].address, 5, 'q'};

  //Get Serial input until ;
  Serial.readBytesUntil(';',prefix,10); 
  
  //Decide what user wants to do and call appropriate functions
  if (String(prefix) == "ChangeCH" || String(prefix) == "ChangePA" || String(prefix) == "ChangeRT")
  {
    Serial.flush();
    //Carlos put change code here
  }
  else if (String(prefix)=="PrintCT") //print connection table
  {
    for (int i=0; i<6; i++)
    {
      printf("%i ", c_t[i].value);
    }
    printf("\n");
    Serial.flush();
  }
  else
  {
    for (int i = 0; i < 6; i++)
    {
      if (String(prefix) == myAddresses[i].userName)
      {
        final_node_index = i;
        to_node_index = checkConnection(final_node_index, my_node_index);
        i = 7;
      }
    }
    if (to_node_index < 6)
    {
       returnHeader = {myAddresses[to_node_index].address, myAddresses[my_node_index].address, myAddresses[final_node_index].address, 5, 'M'}; 
    }
  }
  return returnHeader;
}

// ***Broadcast Message***
void broadcastMessage(void) 
{
  uint8_t broadcastMessage[6];
  
  broadcastMessage[0] = myAddresses[my_node_index].address; // Hardcode contents of broadcastMessage
  broadcastMessage[1] = myAddresses[my_node_index].address; 
  broadcastMessage[2] = myAddresses[my_node_index].address;
  broadcastMessage[3] = 0;
  broadcastMessage[4] = 'B';
  broadcastMessage[5] = channel;
  
  Serial.println(F("Broadcasting...\n"));
  radio.stopListening();
  radio.openWritingPipe(myAddresses[(my_node_index + 1) % 6].address); 
  radio.write(&broadcastMessage, sizeof(broadcastMessage)); //Broadcast message to update connections
  radio.startListening();
}

// ***Check for Connections***
int checkConnection(int final_node_index, int from_node_index)
{
  Serial.println(F("Inside checkConnection"));
  int to_node_index = 6;
  int checkValue = 1;
  if (c_t[from_node_index].value == checkValue)
  {
    checkValue++;
  }
  if (c_t[final_node_index].value != 0)
  {
    to_node_index = final_node_index;
  }
  else
  {
    for (int j = 0; j < 6; j++)
    {
      if (c_t[j].value == checkValue)
      {
        to_node_index = j;
      }
    }
    if (to_node_index == 6)
    {
      Serial.println(F("You are not connected to the mesh."));
    }
  }
  printf("to_node_index: %i\n", to_node_index);
  return to_node_index;
}


void scanChannels() {
  //radio.openWritingPipe(myAddresses[(my_node_index + 1) % 6].address);
  //radio.openReadingPipe(1, myAddresses[0].address);
  //radio.startListening();
  
  //Serial.println("searching for channels...");
  bool found_channel = false;
  int i = 50;
  uint8_t receive_int[6] = {200,200,200,200,200,200};
  
  while (!found_channel)
  {
    radio.setChannel(i);
    radio.startListening();
    delayMicroseconds(128);
    radio.stopListening();
    if (radio.testCarrier())
    {
      printf("Checking message on channel: %d \n\r", i);
      int start_time = millis();
      int end_time = millis();
      radio.openWritingPipe(myAddresses[(my_node_index + 1) % 6].address);
      radio.openReadingPipe(1, myAddresses[0].address);
      radio.startListening();
      while (start_time + 2000 > end_time)
      {
        if (radio.available())
        {
          Serial.println("reading...");
          radio.read(&receive_int, sizeof(receive_int));
          
        }
        end_time = millis();  
      }
      for (int i = 0; i < 5; i++) 
      {
        Serial.println(receive_int[i], HEX);
      }
      Serial.println(receive_int[5]);
      if (receive_int[5] == i)
      {
        found_channel = true;
        channel = i;
        Serial.println("channel set to: " + i);
      }
      else
        Serial.println("Failed to authenticate");
    }
    i--;
    if (i<0)
      i = 50;
  }
}
