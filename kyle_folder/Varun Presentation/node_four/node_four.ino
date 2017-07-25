#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"
#include "printf.h"
#include "EEPROM.h"
#include <string.h>

RF24 radio(9,10);

const uint64_t pipes[5] = { 0xF0F0F0F0A1LL, 0xF0F0F0F0B2LL, 0xF0F0F0F0C3LL,
                            0xF0F0F0F0D4L, 0xF0F0F0F0E5LL };

void setup() {
  Serial.begin(38400);
  printf_begin();
  radio.begin();
  radio.enableDynamicPayloads();
  radio.setRetries(15, 15);
  radio.startListening();
  radio.setChannel(90);

  for (int i = 1; i < 6; i++)
  {
    radio.openReadingPipe(i, pipes[i - 1]);
  }
  radio.startListening();
}

void loop() {
  char scene = '0';
  while (!radio.available()) { } 
  radio.read(&scene, sizeof(scene));
  if (scene == '3')
    sceneThree();
  else 
    delay(10);
    //do nothing
}

void sceneThree() {
  Serial.println("In scene three");
  char message[32] = "";
  memset(message, 0, 32);
  radio.read(&message, sizeof(message));
  //String m = String(message);
  printf("Node1 -> Node4\n\r");
  printf("%s\n\r", message);
}


