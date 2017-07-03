/*
 * Transciever Blink Program
 * Switch is read on the TX module. If switch is pressed, send value to RX.
 * RX will light LED when value is recieved
 * A. Greentree, 6/23/17
 */

#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"
#include "printf.h"


// Set up nRF24L01 radio on SPI bus plus pins 9 & 10 

int tonefreq;

// Radio pipe addresses for the 2 nodes to communicate.
RF24 radio(9,10);
const uint64_t pipe=0xF0F0F0F0E1LL; 

int buzzer=8;

void setup(void)
{
 Serial.begin(57600);
 //Serial.println("Start");
 radio.begin();
 radio.setChannel(120);
 radio.openReadingPipe(1,pipe);
 radio.startListening();
 pinMode(buzzer, OUTPUT);}
 
void loop(void){
 // Serial.println(msg[0]);
 if (radio.available()){
   bool done = false;    
   while (!done){
     done = radio.read(&tonefreq, sizeof(tonefreq));      
     Serial.println(tonefreq);
     tone(buzzer, tonefreq);
     delay(300);
     noTone(buzzer);
     delay(50);
   }
 }
 //else{Serial.println("No radio available");}
}   

