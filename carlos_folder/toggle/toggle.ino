int led = 4;
int button = 3;
int mode = 0;
int oldButtonState = LOW;

void setup() 
{
  pinMode(led, OUTPUT);
  pinMode(button, INPUT);
  attachInterrupt(digitalPinToInterrupt(button), interrupt, HIGH);
}

void loop() 
{ 
  if (mode == 1) {
    digitalWrite(led, HIGH);
  } else
  {
    digitalWrite(led, LOW);
  }
}

void interrupt(void)
{
  int newButtonState = digitalRead(button);
  if (newButtonState == HIGH && oldButtonState == LOW)
  {
    mode = 1;
  }
  else 
  {
    mode = 0;
  }
}

