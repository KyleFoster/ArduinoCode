
/** RF24Mesh_Example_Node2Node.ino by TMRh20
 *
 * This example sketch shows how to communicate between two (non-master) nodes using
 * RF24Mesh & RF24Network
 **/


#include "RF24.h"
#include "RF24Network.h"
#include "RF24Mesh.h"
#include <SPI.h>
#include <printf.h>


//########### USER CONFIG ###########

/**** Configure the nrf24l01 CE and CS pins ****/
RF24 radio(9, 10);
RF24Network network(radio);
RF24Mesh mesh(radio, network);

/**
 * User Configuration:
 * nodeID - A unique identifier for each radio. Allows addressing to change dynamically
 * with physical changes to the mesh. (numbers 1-255 allowed)
 *
 * otherNodeID - A unique identifier for the 'other' radio
 *
 **/
#define nodeID 2

//#################################

uint32_t millisTimer = 0;
uint32_t stringTimer = 0;
char dataStr[] = {"abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890"};
char tmpStr[sizeof(dataStr) + 1];
uint8_t strCtr = 1;

uint32_t delayTime = 120;


//Declare and Initialize Variables
unsigned int sizeCtr = 2;
uint32_t errorCount = 0;
uint32_t duplicates = 0;
uint32_t totalData = 0;
int nodeNum=0;
int16_t nodeAddress=0;
int16_t AddressBook[6];
int i=0; 
int bufferval=0; 
int bufferreturn;

int setAddresses(){
 // printf("----------------------------------\n");

 //Declare and initialize Variables
 int sendNode=0; 
 bool done=false;

  //Check for address of each node, print nodes that are available
  printf("The following other nodes are connected to the network: ");
  for (nodeNum=1; nodeNum<7; nodeNum++){
    nodeAddress=mesh.getAddress(nodeNum);
    bufferval=nodeNum-1;
    if(nodeAddress>0){
      //printf("RF24 Address: 0%o\n", nodeAddress);
      AddressBook[bufferval]=nodeAddress;
      printf(" %i, ", nodeNum);
    }
    else 
      AddressBook[bufferval]=0;
  }
  printf("\n");

  printf("Please enter the node you want to send data to: ");

  while(Serial.available()==0){
    //wait for user
  }
  sendNode=Serial.parseInt();
  
  return sendNode;
}

void setup() {
  Serial.begin(115200);
  //SPI.begin();
  radio.begin();
  printf_begin();
  radio.printDetails();
  // Set the nodeID manually
  mesh.setNodeID(nodeID);
  // Connect to the mesh
  Serial.println(F("Connecting to the mesh..."));
  mesh.begin();
  //network.begin();
}

void loop() {
  //Declare and initialize local variables
  int sendNode=0;
  
  mesh.update(); //update the network

  //Call setAddresses
  sendNode=setAddresses();
  printf("%i\n", sendNode);

  uint16_t recipient_address = AddressBook[sendNode-1];
  uint32_t time;
  bool ok=false;
  RF24NetworkHeader header(recipient_address,'T');
  ok=network.write(header,&time,sizeof(time)); 
  if (ok)
    printf("yay");
  else
    printf("boo");

  delay(500);
}

