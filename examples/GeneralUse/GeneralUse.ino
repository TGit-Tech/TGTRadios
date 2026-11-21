
// These are defined in Ebyte.h but Ebyte runs in it's own translation unit so not accessable outside.
/*********************************************//**
 * @defgroup EM Ebyte Radio Models Supported
 * @{
 ************************************************/
#define EM_NOTGOT        0
#define EM_UNKNOWN       1
#define EM_E22_400T30D   2
#define EM_E22_400T33D   3
#define EM_E22_900T30D   4
#define EM_E32_433T20D   5
#define EM_E32_433T30D   6
#define EM_E220_400T30D  7

/*********************************************//**
 * @defgroup EMS Ebyte Radio Model Series
 * @{
 ************************************************/
#define EMS_UNKNOWN     0
#define EMS_E220E22     1
#define EMS_E32         2

/*********************************************//**
 * @defgroup ERFMODE Ebyte Lora Radio Modes (M0,M1)
 * @{
 ************************************************/
#define ERFNORMMODE  0
#define ERFWAKEMODE  1
#define ERFPWRSAVE   2
#define ERFSETMODE   3
///@}

#include <Ebyte.h>

//#define RFPINS 9,8,14,15,50       //IO-Access backplane RF-Pins *M0*/8, /*M1*/9, /*ATX*/14, /*ARX*/15, /*AUX*/50
Ebyte oEbyte(9,8,14,15,50);

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
