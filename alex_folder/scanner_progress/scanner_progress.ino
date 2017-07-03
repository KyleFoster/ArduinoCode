#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"
#include "printf.h"

RF24 radio(9,10);
const uint64_t pipes[2] = { 0xF0F0F0F0E1LL, 0xF0F0F0F0D2LL };
const uint8_t num_channels = 127;
//uint8_t values[num_channels];
char received_payload[100] = "";
String code;
int i;

void setup(void)
{
  Serial.begin(57600);
  printf_begin();
  printf("\n\rRF24/examples/scanner/\n\r");
  radio.begin();
  radio.setAutoAck(false);
  radio.printDetails();
  radio.startListening();
  radio.stopListening();
}

const int num_reps = 100;
void loop(void)
{
  //memset(values,0,sizeof(values));
//  int rep_counter = num_reps;
//  while (rep_counter--)
//  {
//    int i = num_channels;
//    while (i--)
//    {
//      radio.setChannel(i);
//      radio.startListening();
//      delayMicroseconds(128);
//      radio.stopListening();
//      if ( radio.testCarrier())
//      {
//        printf("%d\n\r", i);
//        join_network(i);
//      }
//    }
//  }
  radio.setChannel(4);
  radio.startListening();
  memset(received_payload, 0, 32);
  int len = radio.getDynamicPayloadSize();
  radio.read(&received_payload, len);
  String m = String(received_payload);
  Serial.println(m);
}

//void join_network(int x)
//{
//  String send_payload = String("Hello");
//  radio.openWritingPipe(pipes[0]);
//  radio.openReadingPipe(1,pipes[1]);
//  radio.stopListening();
//  int start_time = millis();
//  int end_time = millis();
//  code = String("1010");
//  if (radio.write(&send_payload, sizeof(send_payload)))
//  {
//    printf("payload sent\n\r");
//    radio.startListening();
//    while(start_time + 5000 > end_time)
//    {
//      memset(received_payload, 0, 32);
//      int len = radio.getDynamicPayloadSize();
//      radio.read(&received_payload, len);
//      String m = String(received_payload);
//      if (m == code)
//      {
//        radio.setChannel(i);
//        printf("You have joined channel %d MHz\n\r", i); 
//        i = 1000;
//      }
//      end_time = millis();
//    }
//  }
//}

