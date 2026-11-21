// Tested working 2026-11-21
#include <TGTEbyte.h>

// These are the Pins for a Tode with BD241S PCB.
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
/*
virtual void          Begin();
    virtual byte          RadioID();
    virtual void          Mode(byte _Mode);                 ///< SET \ref E32MODES[0]
    virtual byte          Mode();                           ///< GET \ref E32MODES[0]

    virtual void          Address(unsigned int _RFAddress); ///< SET Radio RF-Address[1][2]
    virtual unsigned int  Address();                        ///< GET Radio RF-Address[1][2]

    virtual void          AirSpeed(byte _Speed);            ///< SET Radio AirSpeed[3.012]
    virtual byte          AirSpeed();                       ///< GET Radio AirSpeed[3.012]
    virtual void          UARTSpeed(byte _eSpeed);          ///< SET Radio UART Speed[3.34567]
    virtual byte          UARTSpeed();                      ///< GET Radio UART Speed[3.34567]

    virtual void          Frequency(int _RFFrequency);      ///< SET Radio Frequency Setting[4.01234]
    virtual int           Frequency();                      ///< GET Radio Frequency Setting[4.01234]
    virtual byte          Channel();                        ///< GET Channel and Return to Normal Mode[4.01234]

    virtual void          TxPower(byte _TxPower);           ///< SET Radio Tx Power Setting[5.01]
    virtual byte          TxPower();                        ///< GET Radio Tx Power Setting[5.01]

    virtual void          NetID(byte _NetID);
    virtual byte          NetID();

    virtual void          FixedTransmission(bool _Fixed);
    virtual bool          FixedTransmission();
    virtual void          E32IODrive(bool _Drive);
    virtual bool          E32IODrive();
    virtual void          RSSIOn(bool _On);
    virtual bool          RSSIOn();
    virtual void          LBTOn(bool _On);
    virtual bool          LBTOn();
    virtual void          WUORTime(byte _Time);
    virtual byte          WUORTime();
    virtual void          E32FECOn(bool _On);
    virtual bool          E32FECOn();
    virtual void          E22WOROn(bool _On);
    virtual bool          E22WOROn();
*/
