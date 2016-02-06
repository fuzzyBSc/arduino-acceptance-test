#include <IRremote.h>

// Listen on pin D11
IRrecv irrecv(11);

void setup() {
  irrecv.enableIRIn();
  Serial.begin(9600);
}

int last;

void loop() {
  decode_results results;
  if (irrecv.decode(&results)) {
    switch (results.decode_type)
    {
      case NEC:
        Serial.print("NEC/");
        break;
      case SONY:
        Serial.print("SONY/");
        break;
      case RC5:
        Serial.print("RC5/");
        break;
      case RC6:
        Serial.print("RC6/");
        break;
      case UNKNOWN:
        Serial.print("UNKNOWN/");
        break;
      default:
        Serial.print(results.decode_type, HEX);
        Serial.print("/");
        break;
    }
    Serial.print(results.value, HEX);
    Serial.print(" = ");
    const char  *sep = "";
    for (int ii = 0; ii < results.rawlen; ++ii)
    {
      Serial.print(sep);
      Serial.print(results.rawbuf[ii], HEX);
      sep = ",";
    }
    Serial.println();
    irrecv.resume();
  }
}
