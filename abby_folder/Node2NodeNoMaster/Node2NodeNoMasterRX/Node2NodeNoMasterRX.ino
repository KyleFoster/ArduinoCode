
/** RF24Mesh_Example_Node2Node.ino by TMRh20
 *
 * This example sketch shows how to communicate between two (non-master) nodes using
 * RF24Mesh & RF24Network
 **/


#include "RF24.h"
#include "RF24Network.h"
#include "RF24Mesh.h"
#include <SPI.h>
//#include <printf.h>


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
#define nodeID 0
#define otherNodeID 1

//#################################

uint32_t millisTimer = 0;
uint32_t stringTimer = 0;
char dataStr[] = {"abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890"};
char tmpStr[sizeof(dataStr) + 1];
uint8_t strCtr = 1;

uint32_t delayTime = 120;

struct addressBook{
  String userName;
  uint64_t address;
};
addressBook myAddresses[6]={{"Abby", 0xF0F0F0F0E1LL},{"Carlos", 0xF0F0F0F0D2LL},{"Kyle", 0xF0F0F0F0A1LL},{"Alex", 0xF0F0F0F0B2LL},{"Harman", 0xF0F0F0F0C1LL},{"Malik", 0xF0F0F0F0D1LL}};

void setup() {

  Serial.begin(115200);
  //printf_begin();
  // Set the nodeID manually
  mesh.setNodeID(nodeID);
  // Connect to the mesh
  Serial.println(F("Connecting to the mesh..."));
  mesh.begin();
}

unsigned int sizeCtr = 2;

uint32_t errorCount = 0;
uint32_t duplicates = 0;
uint32_t totalData = 0;

void loop() {
  radio.startListening();
  Serial.println(F("Now Listening"));
    unsigned long start_time = micros();                             // Take the time, and send it.  This will block until complete
     unsigned long got_time;                                 // Grab the response, compare, and send to debugging spew
        radio.read( &got_time, sizeof(unsigned long) );
        unsigned long end_time = micros();
        
        // Spew it
        Serial.print(F("Sent "));
        Serial.print(start_time);
        Serial.print(F(", Got response "));
        Serial.print(got_time);
        Serial.print(F(", Round-trip delay "));
        Serial.print(end_time-start_time);
        Serial.println(F(" microseconds"));
     
}
