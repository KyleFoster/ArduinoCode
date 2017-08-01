int left = 5;
int right = 6;

void setup() {
  
  pinMode(left, OUTPUT);
  pinMode(right, OUTPUT);

}

void loop() {

  ledAlert("route");
  delay(2000);
  ledAlert("offline");
  delay(2000);
  ledAlert("online");
  delay(2000);
  
}

void ledAlert(String mode) 
{
  if (mode == "route") 
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
  else if (mode == "online")
  {
    digitalWrite(left, HIGH);
    digitalWrite(right, HIGH);
  }
  else if (mode == "offline")
  {
    for (int i = 0; i <= 3; i++)
    {
      digitalWrite(left, HIGH);
      digitalWrite(right, HIGH);
      delay(200);
      digitalWrite(right, LOW);
      digitalWrite(left, LOW);
      delay(200);
    }
  }
}
