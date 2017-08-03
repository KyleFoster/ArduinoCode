int left = 5;
int right = 6;

void setup() {
  
  pinMode(left, OUTPUT);
  pinMode(right, OUTPUT);

}

void loop() {

  ledAlert("offline");
  delay(2000);
  
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

