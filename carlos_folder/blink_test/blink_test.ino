int one = 0;
int two = 1;
int three = 2;
int left = 5;
int right = 6;

int nite[5] = {one, two, three, left, right};

void setup() {

  pinMode(one, OUTPUT);
  pinMode(two, OUTPUT);
  pinMode(three, OUTPUT);
  pinMode(left, OUTPUT);
  pinMode(right, OUTPUT);

}

void loop() {

  ledAlert("sweep");
//  delay(1000);
  
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
      digitalWrite(two, HIGH);
      delay(25);
      digitalWrite(two, LOW);
      digitalWrite(left, HIGH);
      delay(45);
      digitalWrite(left, LOW);
      digitalWrite(one, HIGH);
      delay(25);
      digitalWrite(one, LOW);
      digitalWrite(right, HIGH);
      delay(45);
      digitalWrite(right, LOW);
      digitalWrite(three, HIGH);
      delay(25);
      digitalWrite(three, LOW);
    }
  }
  else if (mode == "sweep")
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

