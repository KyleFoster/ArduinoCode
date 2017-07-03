#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"
#include "printf.h"
#include "EEPROM.h"

RF24 radio(9,10);
int i,j,p,r,m,x,mode,rate,power,chan0,chan1;

void setup(void)
{
  Serial.begin(57600);
  Serial.print("");
  printf_begin();
  radio.begin();

  Serial.flush();
  printf("*****BOOZ ALLEN HAMILTON WIFI JAMMER 2000*****\n\r");
  printf("Use saved settings? Type 'Y' for Yes or any key to continue.\n\r");
  
  while(!Serial.available());
    String save = Serial.readString();
    save.toUpperCase();
    if(save == "Y") {
      
      power = EEPROM.read(0); 
      choosePower(power);
      rate = EEPROM.read(1);
      setRate(rate);
      mode = EEPROM.read(2);
      
      }else{
        Serial.flush();
        printf("Choose Power Level:\n\r 0: -18dBm\n\r 1: -12dBm\n\r 2: -6dBm \n\r 3: 0dBm  \n\r");
        while(!Serial.available()); 
          power = Serial.parseInt();
          choosePower(power);

        Serial.flush();
        printf("\n\rChoose Data Rate:\n\r 0: 250kbps\n\r 1: 1Mbps\n\r 2: 2Mbps\n\r");
        while(!Serial.available());
          rate = Serial.parseInt();
          setRate(rate);

        Serial.flush();
        printf("\n\rMode Select:\n\r 0: Static\n\r 1: Scan\n\r");
        while(!Serial.available());
          mode = Serial.parseInt();
        
        Serial.flush();
        printf("\n\rSave setup? Type 'Y' for Yes or any key to continue\n\r");
        while(!Serial.available());
          String saveSetup = Serial.readString();
            saveSetup.toUpperCase();
              if(saveSetup == "Y") {
                EEPROM.write(0,power);
                EEPROM.write(1,rate);
                EEPROM.write(2,mode);
              }
        }
radio.printDetails();
}

void loop(void) {
  
      if(mode == 1){
        printf("\n\rSelect Range:\n\rLower Bound: ");
        Serial.flush();
        while(!Serial.available());
        chan0 = Serial.parseInt();
        x = chan0;
        printf("%i\n\r", chan0);
        printf("Upper Bound: "); 
        Serial.flush();
        while(!Serial.available());
        chan1 = Serial.parseInt();
        printf("%i\n\r", chan1);
        printf("Scanning..."); 
        unsigned long time = millis();
        while(time > 0) {
          for(chan0; chan0 <= chan1; chan0++) {
          radio.setChannel(chan0);
          radio.write(&time, sizeof(unsigned long));  
          if(chan0 == chan1) {
          chan0 = x;
          }
         }
        }
      }else{
        printf("\n\rSelect Channel 0 - 127 to Jam.\n\r");
        Serial.flush();
        while(!Serial.available());
        int chan = Serial.parseInt();
        if(chan <= 127){  
        radio.setChannel(chan);
        printf("Transmitting on %i. \n\r", chan);
        unsigned long time = millis();
        while(time > 0){
        radio.write(&time, sizeof(unsigned long));  
          if(Serial.available()) {
            break; 
          }
         }
        }
       }
      }
    

// ***Choose Power***
void choosePower(int POWER) {
    if(POWER == 0) {
      radio.setPALevel(RF24_PA_MIN);
    }else if(POWER == 1) {
      radio.setPALevel(RF24_PA_LOW);
    }else if(POWER == 2) {
      radio.setPALevel(RF24_PA_HIGH);
    }else if(POWER == 3) {
      radio.setPALevel(RF24_PA_MAX);
    }
}

// ***Set Data Rate***
void setRate(int RATE) {
  if(RATE == 0) {
    radio.setDataRate(RF24_250KBPS);
  }else if(RATE == 1) {
    radio.setDataRate(RF24_1MBPS);
  }else if(RATE == 2) {
    radio.setDataRate(RF24_2MBPS);
  }
}

