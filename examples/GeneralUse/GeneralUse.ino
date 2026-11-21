#include <TGTEbyte.h>

//#define RFPINS 9,8,14,15,50       
TGTEbyte oEbyte(Serial3,/*M0*/9,/*M1*/8,/*AUX*/50);

void setup() {
  Serial.begin(115200);
  Serial.println(F("TGTRadios Ebyte Test"));

  Serial3.begin(9600);
  oEbyte.Begin();

  uint8_t RadioModel = oEbyte.RadioID();
  Serial.println("");Serial.println("");Serial.println("Radio Identity is ... ");
  switch(RadioModel) {
    case 0: Serial.println("EM_NOTGOT");break;
    case 1: Serial.println("EM_UNKNOWN");break;
    case 2: Serial.println("EM_E22_400T30D");break;
    case 3: Serial.println("EM_E22_400T33D");break;
    case 4: Serial.println("EM_E22_900T30D");break;
    case 5: Serial.println("EM_E32_433T20D");break;
    case 6: Serial.println("EM_E32_433T30D");break;
    case 7: Serial.println("EM_E220_400T30D");break;
  }
  Serial.println("");Serial.println("");
}

void loop() {
  // put your main code here, to run repeatedly:

}
