int left = 4;
int red = 5;
int white = 6;
int blue = 7;
int right = 8;

int interruptPin = 3;
int light = 8;
volatile byte state = LOW;

int nite[5] = {red, white, blue, left, right};

void setup() {

  pinMode(red, OUTPUT);
  pinMode(white, OUTPUT);
  pinMode(blue, OUTPUT);
  pinMode(left, OUTPUT);
  pinMode(right, OUTPUT);

  pinMode(light, OUTPUT);
  pinMode(interruptPin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(interruptPin), smoke, LOW);

}

void loop() {

//  digitalWrite(light, state);
//  ledAlert("broadcast");
//  delay(3000);
//  ledAlert("routing");
//  delay(3000);
  digitalWrite(light, state);
  ledAlert("routing");
  delay(1000);
  
}

void smoke() 
{
  digitalWrite(light, HIGH);
  delayMicroseconds(500000);
  digitalWrite(light, LOW);
}

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
  else if (mode == "relay2")
  {
    for(int i = 0; i <= 8; i++)
    {
      digitalWrite(white, HIGH);
      delay(25);
      digitalWrite(white, LOW);
      digitalWrite(left, HIGH);
      delay(45);
      digitalWrite(left, LOW);
      digitalWrite(red, HIGH);
      delay(25);
      digitalWrite(red, LOW);
      digitalWrite(right, HIGH);
      delay(45);
      digitalWrite(right, LOW);
      digitalWrite(blue, HIGH);
      delay(25);
      digitalWrite(blue, LOW);
    }
  }
  else if (mode == "routing")
  {
    for (int i = 0; i <= 6; i++)
    {
      for (int i = 0; i <= 4; i++)
      {
        digitalWrite(nite[i], HIGH);
        delay(20);
        digitalWrite(nite[i], LOW);
      }
      delay(10);
      for (int i = 4; i >= 0; i--)
      {
        digitalWrite(nite[i], HIGH);
        delay(20);
        digitalWrite(nite[i], LOW);
      }
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

