#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"
#include "printf.h"

RF24 radio(9,10);
const uint64_t pipes[2] = { 0xF0F0F0F0E1LL, 0xF0F0F0F0D2LL };
const uint8_t num_channels = 128;
uint8_t values[num_channels];
char received_payload[100] = "";
String code;
int i;
String m;

void setup(void)
{
  Serial.begin(57600);
  printf_begin();
  radio.begin();
  radio.printDetails();
  radio.startListening();
  radio.stopListening();
}

const int num_reps = 100;
void loop(void)
{
  memset(values,0,sizeof(values));
  int rep_counter = num_reps;
  while (rep_counter--)
  {
    int i = num_channels;
    while (i--)
    {
      radio.setChannel(i);
      radio.startListening();
      delayMicroseconds(128);
      radio.stopListening();
      if ( radio.testCarrier())
      {
        printf("%d\n\r", i);
        receiveMessage(i);
      }
    }
  }
}
 
//void authenticate(int x)
//{
//  receiveMessage();
//  Serial.println(m);
//  if (code == "OK")
//  {
//    printf("You are now set to %d MHz\n\r", x);
//    x = 1000;
//  }
//}

void receiveMessage(int x) {
  Serial.println("receive");
  int start_time = millis();
  int end_time = millis(); 
  while(start_time + 5000 > end_time)
  {
    if (!radio.available())
    {
      radio.setChannel(x);
      printf("You are now set to %d MHz\n\r", x);
      end_time += 5000;
    }
    end_time = millis();
  }  
}

