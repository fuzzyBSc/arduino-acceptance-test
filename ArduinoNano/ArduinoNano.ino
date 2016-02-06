

enum TestStatus {
  YES,
  NO,
  PASS,
  FAIL
};

/*
   Pin configuration. Adjust to your model if needed.
*/

// How many digital pins do you have?
const int digitalPinCount = 14;
// How many analogue pins do you have?
const int analoguePinCount = 8;

// Test pins 2-13 for digital out functionality
TestStatus digitalOutPins[] = {
  NO, NO, YES, YES, YES,
  YES, YES, YES, YES, YES,
  YES, YES, YES, YES
};

// Test pins 2-13 for digital in functionality
TestStatus digitalInPins[] = {
  NO, NO, YES, YES, YES,
  YES, YES, YES, YES, YES,
  YES, YES, YES, YES
};

// Test pins 2-3 for interrupt functionality
TestStatus interruptPins[] = {
  NO, NO, YES, YES, NO,
  NO, NO, NO, NO, NO,
  NO, NO, NO, NO
};

// Test pins 3,5,6,9,10,11 for pulse width modulation functionality
TestStatus pwmPins[] = {
  NO, NO, NO, YES, NO,
  YES, YES, NO, NO, YES,
  YES, YES, NO, NO
};

// Test pins 0-7 for analogue input functionality
TestStatus analogueInPins[] = {
  YES, YES, YES, YES,
  YES, YES, YES, YES
};
/*
   Helper functions
*/

boolean readSkip()
{
  boolean result = false;
  // Read and flush buffer
  while (Serial.available())
  {
    char key = Serial.read();
    switch (key)
    {
      case 's':
      case 'S':
        result = true;
        break;
      default:
        break;
    }
  }
  return result;
}

int readBoolean(boolean block)
{
  int result = 2;
  while (result == 2 && (block || Serial.available()))
  {
    char key = Serial.read();
    switch (key)
    {
      case 'y':
      case 'Y':
        result = 1;
        break;
      case 'n':
      case 'N':
      case 's':
      case 'S':
        result = 0;
      default:
        // keep looping
        break;
    }
  }
  if (result != 2) {
    while (Serial.available())
    {
      // Flush serial buffer
      Serial.read();
    }
  }
  return result;
}

void printResult(const char *component, int pin, TestStatus result)
{
  if (result == NO) return;

  Serial.print(component);
  Serial.print(" ");
  if (pin >= 0)
  {
    Serial.print("pin ");
    Serial.print(pin);
    Serial.print(" ");
  }
  switch (result)
  {
    case PASS:
      Serial.println("PASS");
      break;
    case FAIL:
      Serial.println("FAIL");
      break;
    case YES:
      // The test was not performed
      Serial.println("SKIPPED");
      break;
    default:
      Serial.println("UNEXPECTED TEST STATUS");
      break;
  }
}

/*
   Interrupt testing
*/

volatile boolean interruptOccurred = false;
void interruptHandler()
{
  interruptOccurred = true;
}
boolean testInterrupt(int pin, int mode)
{
  interruptOccurred = false;
  attachInterrupt(digitalPinToInterrupt(pin), interruptHandler, mode);
  boolean pass = false;
  for (;;)
  {
    if (interruptOccurred)
    {
      pass = true;
      break;
    }
    if (readSkip())
    {
      break;
    }
  }
  detachInterrupt(digitalPinToInterrupt(pin));
  return pass;
}
void testInterrupt(int pin)
{
  if (interruptPins[pin] == YES)
  {
    int pass = true;
    Serial.print("Interrupt test: Touch pin D");
    Serial.print(pin);
    Serial.println(" to 5V. \"s\" to skip");
    pass = pass && testInterrupt(pin, CHANGE);
    Serial.println("Now to ground, then 5V again");
    pass = pass && testInterrupt(pin, RISING);
    Serial.println("Now to ground");
    pass = pass && testInterrupt(pin, LOW);
    Serial.println("Now to 5V then ground again");
    pass = pass && testInterrupt(pin, FALLING);
    Serial.println("Interrupt test finished");
    if (pass)
    {
      interruptPins[pin] = PASS;
    }
  }
}

/*
   Digital input testing
*/

boolean testDigitalIn(int pin, int expectedValue)
{
  boolean pass = false;
  int last = 3;
  for (;;)
  {
    int value = digitalRead(pin);
    if (value != last)
    {
      Serial.println(value);
      last = value;
    }
    if (value == expectedValue)
    {
      pass = true;
      break;
    }
    if (readSkip())
    {
      break;
    }
  }
  return pass;
}
void testDigitalIn(int pin)
{
  if (digitalInPins[pin] == YES)
  {
    // Attach the pull-up resistor
    pinMode(pin, INPUT_PULLUP);
    int pass = true;
    Serial.print("Digital In test: Touch pin D");
    Serial.print(pin);
    Serial.println(" to ground. \"s\" to skip");
    pass = pass && testDigitalIn(pin, LOW);
    Serial.println("Now release or touch to 3.3V");
    pass = pass && testDigitalIn(pin, HIGH);
    if (pass)
    {
      digitalInPins[pin] = PASS;
    }
  }
}

/*
   Analogue input testing
*/
boolean testAnalogueIn(int pin, int expectedValue, int expectedError)
{
  boolean pass = false;
  int last = 4096;
  for (;;)
  {
    int value = analogRead(pin);
    int delta = abs(value - last);
    if (delta > expectedError)
    {
      Serial.println(value);
      last = value;
    }
    int error = abs(value - expectedValue);
    if (error < expectedError)
    {
      Serial.print("Expected ");
      Serial.println(expectedValue);
      Serial.print("Got ");
      Serial.println(value);
      if (expectedValue != value)
      {
        Serial.println("Close enough");
      }
      pass = true;
      break;
    }
    if (readSkip())
    {
      break;
    }
  }
  return pass;
}
void testAnalogueIn(int pin)
{
  if (analogueInPins[pin] == YES)
  {
    int pass = true;
    Serial.print("Analogue In test: Touch pin A");
    Serial.print(pin);
    Serial.println(" to ground. \"s\" to skip");
    pass = pass && testAnalogueIn(pin, 0, 1024 / 5);
    Serial.print("Now touch pin A");
    Serial.print(pin);
    Serial.println(" to 5V");
    pass = pass && testAnalogueIn(pin, 1023, 1024 / 5);
    Serial.print("Now touch pin A");
    Serial.print(pin);
    Serial.println(" to 3.3V");
    pass = pass && testAnalogueIn(pin, 675, 1024 / 5);
    if (pass)
    {
      analogueInPins[pin] = PASS;
    }
  }
}

/*
   Digital output testing
*/

void testDigitalOut(int pin)
{
  if (digitalOutPins[pin] == YES)
  {
    // Activate output on this pin for the duration of the test only
    for (int ii=0; ii<digitalPinCount; ++ii)
    {
      if (digitalOutPins[pin] == YES)
      {
        pinMode(ii, OUTPUT);
      }
    }
    Serial.print("Attach your current-limited LED to pin D");
    Serial.print(pin);
    Serial.println(". Is it blinking?");
    int lastChange_ms = 0;
    boolean lastState = false;
    int pass = 2;
    for (;;)
    {
      int now_ms = millis();
      if (now_ms - lastChange_ms > 500)
      {
        lastChange_ms = now_ms;
        lastState = !lastState;
        for (int ii=0; ii<digitalPinCount; ++ii)
        {
          if (digitalOutPins[pin] == YES)
          {
            digitalWrite(ii, lastState ? HIGH : LOW);
          }
        }
      }
      pass = readBoolean(false);
      if (pass != 2)
      {
        break;
      }
    }
    for (int ii=0; ii<digitalPinCount; ++ii)
    {
      if (digitalOutPins[pin] == YES)
      {
        pinMode(ii, INPUT);
      }
    }
    if (pass)
    {
      digitalOutPins[pin] = PASS;
    }
  }
}

void testPWM(int pin)
{
  if (pwmPins[pin] == YES)
  {
    // Activate output pins for the duration of the test only
    for (int ii=0; ii<digitalPinCount; ++ii)
    {
      if (pwmPins[pin] == YES)
      {
        pinMode(ii, OUTPUT);
      }
    }
    Serial.print("Attach your current-limited LED to pin D");
    Serial.print(pin);
    Serial.println(". Is it breathing?");
    int lastChange_ms = millis();
    int level = 0;
    int increment = 1;
    int pass = 2;
    for (;;)
    {
      for (int ii=0; ii<digitalPinCount; ++ii)
      {
        if (pwmPins[pin] == YES)
        {
          analogWrite(ii, level);
        }
      }
      int now_ms = millis();
      if (now_ms - lastChange_ms > 4) {
        lastChange_ms = now_ms;
        level += increment;
        if (level == 0 || level == 255)
        {
          increment = -increment;
        }
      }
      pass = readBoolean(false);
      if (pass != 2)
      {
        break;
      }
    }
    for (int ii=0; ii<digitalPinCount; ++ii)
    {
      if (pwmPins[pin] == YES)
      {
        pinMode(ii, INPUT);
      }
    }
    if (pass)
    {
      pwmPins[pin] = PASS;
    }
  }
}

void setup()
{
  for (int ii = 0; ii < digitalPinCount; ++ii)
  {
    /*
      Set all pins to input for safety.
      Accidentally connecting ground to a high output pin or 5V to
      a low output pin would damage the microcontroller.
      We also don't want to exceed the maximum current rating of the
      chip at any time.
      Since we're asking the user to touch various wires to each other
      We'll only turn on output for one pin at a time as needed.
    */
    pinMode(ii, INPUT_PULLUP);
  }
  Serial.begin(9600);
}

void loop()
{
  Serial.println("Starting tests.");
  Serial.println();

  Serial.println("Digital Input");
  Serial.println();
  for (int ii = 0; ii < digitalPinCount; ++ii)
  {
    testDigitalIn(ii);
  }

  Serial.println("Digital Interrupt");
  Serial.println();
  for (int ii = 0; ii < digitalPinCount; ++ii)
  {
    testInterrupt(ii);
  }

  Serial.println("Analogue Input");
  Serial.println();
  for (int ii = 0; ii < analoguePinCount; ++ii)
  {
    testAnalogueIn(ii);
  }

  Serial.println("Digital Output");
  Serial.println("You will need a LED attached to a current limiting resistor");
  Serial.println("If you don't have one or do not know what I'm talking about");
  Serial.println("do not proceed");
  Serial.println("WARNING - Touching output pins directly to ground will result in damage to your board");
  Serial.println("WARNING - Driving more than about 8 LEDs simultaneously will result in damage to your board");
  Serial.println("Proceed with digital output tests?");
  Serial.println();
  boolean proceed = readBoolean(true);
  if (proceed)
  {
    for (int ii = 0; ii < digitalPinCount; ++ii)
    {
      testDigitalOut(ii);
    }
    for (int ii = 0; ii < digitalPinCount; ++ii)
    {
      testPWM(ii);
    }
  }

  Serial.println("Test completed.");
  // If you can see this message, serial is working fine
  printResult("Serial", -1, PASS);
  for (int ii = 0; ii < digitalPinCount; ++ii)
  {
    printResult("Digital In", ii, digitalInPins[ii]);
  }
  for (int ii = 0; ii < digitalPinCount; ++ii)
  {
    printResult("Digital Interrupt", ii, interruptPins[ii]);
  }
  for (int ii = 0; ii < analoguePinCount; ++ii)
  {
    printResult("Analogue Input", ii, analogueInPins[ii]);
  }

  for (int ii = 0; ii < digitalPinCount; ++ii)
  {
    printResult("Digital Out", ii, digitalOutPins[ii]);
  }
  for (int ii = 0; ii < digitalPinCount; ++ii)
  {
    printResult("Pulse Width Modulation", ii, pwmPins[ii]);
  }
  delay(10000);
}

