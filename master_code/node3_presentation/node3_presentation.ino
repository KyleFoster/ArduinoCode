 /**
   Wireless Mesh Network Protocol.
   @version 42.0
   @authors The Guardian Angels
*/

//Libraries
#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"
#include "printf.h"
#include "EEPROM.h"
#include <string.h>

RF24 radio(9, 10);

#define my_node_index 3 //Change this to your respective address index 
int left = 5;
int right = 6;

//Structs
struct addressBook {
  String userName;
  uint8_t address;
} myAddresses[6] = {{"Troop", 0xA1}, {"Two", 0xB1}, {"Three", 0xC1}, {"Four", 0xD1}, {"Home", 0xE1}, {"Malik", 0xF1}};

struct RadioHeader {
  uint8_t to_address;
  uint8_t from_address;
  uint8_t final_address;
  uint8_t ttl;
  char message_type;
} receiveHeader = {0, 0, 0, 5, '\0'};

struct ConnectionTable
{
  uint8_t value;
  uint8_t updates_til_reset;
  bool disconnected;
} c_t[6] = {{0, 10, false}, {0, 10, false}, {0, 10, false}, {0, 10, true}, {0, 10, true}, {0, 10, true}};

uint8_t channel;
uint8_t received_message[32];

//Function Prototypes
//void setup()
//void loop()
//void sendMessage(RadioHeader &sendHeader)
//void receiveMessage()
//RadioHeader readUserInput()
//int checkConnection(int final_node_index, int from_node_index)

/******************************************setup()****************************************************/
void setup()
{
  Serial.begin(9600);
  printf_begin();
  radio.begin();
  radio.enableDynamicPayloads();
  randomSeed(analogRead(0));
  radio.setChannel(50);
  radio.setRetries(1,5);
  radio.setPALevel(RF24_PA_MAX);
  radio.setDataRate(RF24_250KBPS);
  radio.printDetails();
  radio.setAutoAck(false);

  pinMode(left, OUTPUT);
  pinMode(right, OUTPUT);

  int j = 0;
  for(int i = 1; i < 6; i++){
    if(j == my_node_index)
      j++;
    radio.openReadingPipe(i, myAddresses[j].address);
    j++;
  }
  radio.startListening();
  
  Serial.println(F("--------------------------------------------------------------------------"));
  /* Print out contact list */
  Serial.println(F("Address Book:\n\r"));
  for (int i = 0; i < 6; i++)
  {
    printf("%s\n", myAddresses[i].userName.c_str());
  }

  Serial.println(F("\n\r- To Send a message:"));
  Serial.println(F("Type user name, semicolon, then message.\n\reg: 'Alex;Hello Alex.'"));
  Serial.println(F("------------Begin Chat-------------"));

  while(Serial.available()){
    Serial.read();
  }
}
/*****************************************************************************************************/


/****************************************loop()*******************************************************/
void loop()
{
  RadioHeader sendHeader;
  bool already_broadcast = false;
  unsigned long previousMillis = 0;
  int random_interval = random(0, 5000);
  unsigned long currentMillis = millis();

  while (!radio.available()) // Wait for incoming data/message
  {
    sendHeader = {0, 0, 0, 5, '\0'};
    currentMillis = millis(); // Start counting for broadcast timer
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
      if (currentMillis - previousMillis > 5000)
      {
        already_broadcast = false;
        previousMillis = currentMillis;
        random_interval = random(0, 5000);
      }
      if (currentMillis - previousMillis > random_interval && !already_broadcast)
      {
        broadcastMessage();
        already_broadcast = true;
      }
    }
  }
  receiveMessage();
}
/*****************************************************************************************************/


/************************************receiveMessage()*************************************************/
void receiveMessage()
{
  int final_node_index = 6, to_node_index = 6, from_node_index = 6;
  RadioHeader receive_header = {0, 0, 0, 5, '\0'};

  memset(received_message, 0, 32);
  radio.read(&received_message, sizeof(received_message));
  receive_header = {received_message[0], received_message[1], received_message[2], received_message[3], received_message[4]};

  //Check that we are receiving messages, delete later
  /*
  Serial.println("Addresses in receive message");
  Serial.print(receive_header.to_address, HEX);
  Serial.print(receive_header.from_address, HEX);
  Serial.print(receive_header.final_address, HEX);
  Serial.print(receive_header.ttl);
  Serial.print(receive_header.message_type);
  Serial.print("\n");
  */
  
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
/*****************************************************************************************************/


/************************************messageDecide()**************************************************/
void messageDecide(RadioHeader &message_header, int to_node_index, int from_node_index, int final_node_index)
{
  //Check for garbage first
  if (message_header.message_type != 'M' && message_header.message_type != 'B')
  { 
    radio.begin();
    radio.enableDynamicPayloads();
    radio.setChannel(50);
    radio.setRetries(1,5);
    radio.setAutoAck(false);
    radio.setDataRate(RF24_250KBPS);

    int j = 0;
    for(int i = 1; i < 6; i++)
    {
      if(j == my_node_index)
        j++;
      radio.openReadingPipe(i, myAddresses[j].address);
      j++;
    }
    radio.startListening();
  }
  //if the message is to you
  else if (message_header.to_address == myAddresses[my_node_index].address)
  {
    if (message_header.final_address == myAddresses[my_node_index].address && message_header.message_type == 'M') //Message for you to read
    {
      printf("%s: ", myAddresses[from_node_index].userName.c_str());
      for (int i = 5; i < 32; i++)
      {
        printf("%c", received_message[i]);
      }
      printf("\n");
      ledAlert("receive");
    }
    else if (message_header.final_address != myAddresses[my_node_index].address && message_header.message_type == 'M') //Message you need to relay
    {
      if (message_header.ttl > 0)
        relayMessage(final_node_index, from_node_index);
    }
  }
  else
  {
    if (message_header.message_type == 'B' && message_header.from_address != myAddresses[my_node_index].address) //Broadcast Message, update the connection table
    {
      if (!c_t[from_node_index].disconnected)
        updateTable(from_node_index);
      else if (from_node_index == 3 || from_node_index == 2)
        Serial.println("ignoring broadcast from node: " + from_node_index);
      
      /*
      else
         int x=5;
        Serial.println("currently disconnected from " + myAddresses[from_node_index].userName);
      */
    }

  }
}
/*****************************************************************************************************/


/*****************************************relayMessage()**********************************************/
void relayMessage(int final_node_index, int from_node_index)
{
  ledAlert("relay");
  int to_node_index = checkConnection(final_node_index, from_node_index);
  received_message[0] = myAddresses[to_node_index].address; // change to_address to your best connection
  received_message[3]--; // decrement ttl
  radio.stopListening();
  radio.openWritingPipe(myAddresses[my_node_index].address);
  radio.write(&received_message, sizeof(received_message));
  radio.startListening();
}
/*****************************************************************************************************/


/******************************************updateTable()****************************************************/
void updateTable(int table_index)
{
  bool has_five = false;
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
}
/*****************************************************************************************************/


/*************************************sendMessage()***************************************************/
void sendMessage(RadioHeader &sendHeader)
{
  int final_node_index;
  char send_payload[32] = "";
  uint8_t totalMessage[32];

  memset(send_payload, 0, 32);
  memset(totalMessage, 0, 32);

  Serial.readBytesUntil('\n', send_payload, 32);

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
      i = 7;
    }
  }

  radio.stopListening();
  radio.openWritingPipe(myAddresses[my_node_index].address);
  radio.write(&totalMessage, sizeof(totalMessage));
  Serial.print(myAddresses[my_node_index].userName);
  Serial.print(" -> ");
  Serial.print(myAddresses[final_node_index].userName);
  Serial.print(": ");
  Serial.println(send_payload);
  radio.startListening();
  while(Serial.available()){
    Serial.read();
  }
}
/*****************************************************************************************************/


/********************************************readUserInput()******************************************/
RadioHeader readUserInput()
{
  char prefix[20];
  int final_node_index = 6, to_node_index = 6;
  RadioHeader returnHeader = {myAddresses[my_node_index].address, myAddresses[my_node_index].address, myAddresses[my_node_index].address, 5, 'q'};

  memset(prefix, 0, 20);
  Serial.readBytesUntil(';', prefix, 15);
  String p = String(prefix);

  //Decide what user wants to do and call appropriate functions
  if (p == "ChangeCH" || p == "ChangePA" || p == "ChangeRT")
  {
    //Carlos put change code here
  }
  else if (p == "PrintCT") //print connection table
  {
    Serial.println(F("You are connected to: "));
    for (int i = 0; i < 6; i++)
    {
      if(c_t[i].value>0)
        printf("  %s\n", myAddresses[i].userName.c_str());
    }
  }
  else if (p == "RemoveAbby" || p == "RemoveCarlos" || p == "RemoveKyle" ||
           p == "RemoveAlex" || p == "RemoveHarman" || p == "RemoveMalik") //disconnect from the person 
  {
    //Serial.println("Going to disconnectFrom()");
    disconnectFrom(p);  
  }
  else if (p == "ReconnectAbby" || p == "ReconnectCarlos" || p == "ReconnectKyle" ||
           p == "ReconnectAlex" || p == "ReconnectMalik" || p == "ReconnectHarman")
  {
    //Serial.println("Going to reconnectTo()");
    reconnectTo(p);
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
/*****************************************************************************************************/


/********************************broadcastMessage()***************************************************/
void broadcastMessage()
{
  uint8_t broadcastMessage[6];

  broadcastMessage[0] = myAddresses[my_node_index].address; // Hardcode contents of broadcastMessage
  broadcastMessage[1] = myAddresses[my_node_index].address;
  broadcastMessage[2] = myAddresses[my_node_index].address;
  broadcastMessage[3] = 0;
  broadcastMessage[4] = 'B';
  broadcastMessage[5] = channel;

  radio.stopListening();
  radio.openWritingPipe(myAddresses[my_node_index].address);
  radio.write(&broadcastMessage, sizeof(broadcastMessage)); //Broadcast message to update connections
  radio.startListening();

  ledAlert("broadcast");
}
/*****************************************************************************************************/


/***********************************checkConnection()*************************************************/
int checkConnection(int final_node_index, int from_node_index)
{
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
      ledAlert("offline");
      Serial.println(F("You are not connected to the mesh."));
    }
  }

  return to_node_index;
}
/*****************************************************************************************************/


/***********************************disconnectFrom()**************************************************/
void disconnectFrom(String person) {
  if (person.length() == 10)
  {
    if (person[6] == 'K') // disconnect from kyle
    {
      Serial.println("disconnect from kyle");
      c_t[2].value = 0;
      c_t[2].disconnected = true; 
    }
    else if (person[7] == 'l') //disconnect from alex
    {
      Serial.println("disconnect from alex");
      c_t[3].value = 0;
      c_t[3].disconnected = true; 
    }
    else //disconnect from abby
    {
      Serial.println("disconnect from abby");
      c_t[0].value = 0;
      c_t[0].disconnected = true; 
    }
  }
  else if (person.length() == 12)
  {
    if (person[6] == 'C') // Disconnect from carlos
    {
      Serial.println("disconnect from carlos");
      c_t[1].value = 0;
      c_t[1].disconnected = true;  
    }
    else //disconnect from harman
    {
      Serial.println("disconnect from harman");
      c_t[4].value = 0;
      c_t[4].disconnected = true;  
    }
  }
  else // disconnect from Malik
  {
    Serial.println("disconnect from malik");
    c_t[5].value = 0;
    c_t[5].disconnected = true;  
  }
}
/*****************************************************************************************************/


/***********************************reconnectTo()****************************************************/
void reconnectTo(String person) {
  if (person.length() == 13)
  {
    if (person[9] == 'K')       // Disconnect from kyle
    {
      Serial.println("reconnect to kyle");
      c_t[2].disconnected = false; 
    }
    else if (person[10] == 'l')  // Disconnect from alex
    {
      Serial.println("reconnect to alex");
      c_t[3].disconnected = false; 
    }
    else //disconnect from abby
    {
      Serial.println("reconnect to abby");
      c_t[0].disconnected = false;
    } 
  }
  else if (person.length() == 15)
  {
    if (person[9] == 'C') // Disconnect from carlos
    {
      Serial.println("reconnect to carlos");
      c_t[1].disconnected = false;  
    }
    else                  // Disconnect from harman
    {
      Serial.println("reconnect to harman");
      c_t[4].disconnected = false;  
    }
  }
  else // Disconnect from Malik
  {
    Serial.println("reconnect to malik");
    c_t[5].disconnected = false;
  }  
}
/*****************************************************************************************************/


/***********************************ledAlert()********************************************************/
void ledAlert(String mode) 
{
  if (mode == "relay") 
  {
    for (int i = 0; i <= 10; i++) 
    {
      digitalWrite(left, HIGH);
      delay(45);
      digitalWrite(left, LOW);
      digitalWrite(right, HIGH);
      delay(45);
      digitalWrite(right, LOW);
    }
  } 
  else if (mode == "receive")
  {
    for (int i = 0; i <= 3; i++) 
    {
    digitalWrite(left, HIGH);
    digitalWrite(right, HIGH);
    delay(150);
    digitalWrite(left, LOW);
    digitalWrite(right, LOW);
    delay(100);
    }
  }
  else if (mode == "offline")
  {
    for (int i = 0; i <= 4; i++)
    {
      for (int i = 0; i <= 255; i++)
      {
        analogWrite(left, i);
        analogWrite(right, i);
        delayMicroseconds(500);
      }
      for (int i = 255; i >= 0; i--)
      {
        analogWrite(left, i);
        analogWrite(right, i);
        delayMicroseconds(500);
      }
    }
  }
  else if (mode == "broadcast")
  {
    digitalWrite(left, HIGH);
    digitalWrite(right, HIGH);
    delay(100);
    digitalWrite(left, LOW);
    digitalWrite(right, LOW);
  }
}
/*****************************************************************************************************/
