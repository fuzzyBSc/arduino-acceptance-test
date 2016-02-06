
int triggerPin = 2;
int echoPin = 3;

void setup() {
  pinMode(triggerPin, OUTPUT);
  pinMode(echoPin, INPUT);
  
  pinMode(13, OUTPUT);
  Serial.begin(9600);
}

long HCSR04_blocking_cm(int triggerPin, int echoPin)
{
  // Pulse trigger
  digitalWrite(2, HIGH);
  delayMicroseconds(10);
  digitalWrite(2, LOW);

  // Time result
  long duration_us = pulseIn(echoPin, HIGH, 100000);
  return duration_us / 58;
}

float movingAverage = 0;
const float exponentialFilter = 0.3;

void loop() {
  // Trigger on
  int cm = HCSR04_blocking_cm(triggerPin, echoPin);
  movingAverage = movingAverage * (1-exponentialFilter) + cm * exponentialFilter;
  Serial.print(movingAverage);
  Serial.print("cm/");
  Serial.print(cm);
  Serial.println("cm");
}

