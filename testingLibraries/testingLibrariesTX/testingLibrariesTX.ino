#include <SPI.h>
#include <stdio.h>
#include "nRF24L01.h"
#include "RF24.h"
#include "RF24Network.h"
#include "RF24Mesh.h"
#include "printf.h"

RF24 radio(9, 10); //Declares CE and CS Pins
RF24Network network(radio); //What does this do??
RF24Mesh mesh(radio, network); //What does this do??

//Addresses

struct addressBook{
  String userName;
  uint64_t address;
};
addressBook myAddresses[2]={{"Abby", 0xF0F0F0F0E1LL},{"Carlos", 0xF0F0F0F0D2LL}};


void setup() {
  Serial.begin(57600);
  printf_begin();
  mesh.begin();

  uint8_t i = 2;
  for(i=0; i<2; i++){
    Serial.print(myAddresses[i].userName);
    printf("%12X\n", myAddresses[i].address);
    radio.openReadingPipe((i+1), myAddresses[i].address);
  }
  radio.startListening();
  
}

void loop() {
  radio.stopListening();
  radio.openWritingPipe(myAddresses[0].address);
  uint32_t displayTimer = 0;
  displayTimer = millis();
  mesh.write(&displayTimer, 'M', sizeof(displayTimer));

  /*
  while (network.available()) {
    RF24NetworkHeader header;
    payload_t payload;
    network.read(header, &payload, sizeof(payload));
    Serial.print("Received packet #");
    Serial.print(payload.counter);
    Serial.print(" at ");
    Serial.println(payload.ms);
  }
  */
}
