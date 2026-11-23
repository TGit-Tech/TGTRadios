#include "TGTEbyte.h"
//#####################################################################################################################
TGTEbyte::TGTEbyte(HardwareSerial& _SPort, byte _PinM0, byte _PinM1, byte _PinAUX)
: TGTRadioI(), SPort(_SPort), PinM0(_PinM0), PinM1(_PinM1), PinAUX(_PinAUX) {
    DBLENTER(("TGTTGTEbyte::TGTEbyte"))
}
//---------------------------------------------------------------------------------------------------------------------
void TGTEbyte::Begin() {
    if ( PinM0 != 0 ) pinMode(PinM0, OUTPUT); 
    if ( PinM1 != 0 ) pinMode(PinM1, OUTPUT);
    if ( PinAUX != 0 ) pinMode(PinAUX, INPUT_PULLUP);
    
    CheckRadio();  
}
//-----------------------------------------------------------------------------------------------------
byte TGTEbyte::RadioID() { return EM; }
//-----------------------------------------------------------------------------------------------------
void TGTEbyte::Address(unsigned int _RFAddress) {
  if (EM<=EM_UNKNOWN) {DBLERROR(("TGTEbyte::GetParam() EM<=EM_UNKNOWN\n")) return; }
  byte CurrentMode=Mode();                                        // Check Radio is Identified
  if (CurrentMode!=ERFSETMODE) Mode(ERFSETMODE);                  // Radio in SETMODE
  if ( EParamGotBytes==0 ) GetParam();                            // Get Parameters if needed.

  paramADDH = highByte(_RFAddress);                     // Set bytes[12]
  paramADDL = lowByte(_RFAddress);                      // Set bytes[12]
  //EEPROM.put(0, _RFAddress);                                    // Save new address in EEPROM too.
  SetParam();                                                   // Save
  Mode(CurrentMode);                                            // Return to last Mode RF was set to.
}
//-----------------------------------------------------------------------------------------------------
unsigned int TGTEbyte::Address() {
  if ( EParamGotBytes==0 ) GetParam();                            // Get Parameters
  return word(paramADDH, paramADDL);            // bytes[12]
}
//-----------------------------------------------------------------------------------------------------
void TGTEbyte::NetID(byte _NetID) {
  if (EM<=EM_UNKNOWN) {DBLERROR(("TGTEbyte::GetParam() EM<=EM_UNKNOWN\n")) return; }
  if (EM==EM_E22_400T30D||EM==EM_E22_400T33D||EM==EM_E22_900T30D) { // Only supported on E22 not E220/32
    byte CurrentMode=Mode();                                        // Check Radio is Identified
    if (CurrentMode!=ERFSETMODE) Mode(ERFSETMODE);                  // Radio in SETMODE
    if ( EParamGotBytes==0 ) GetParam();                            // Get Parameters if needed. 
    paramNETID = _NetID;
    SetParam();                                                     // Write Parameters
    Mode(CurrentMode);                                              // Return to Current Mode
  }
}
//-----------------------------------------------------------------------------------------------------
byte TGTEbyte::NetID() {
  if (EM<=EM_UNKNOWN) {DBLERROR(("TGTEbyte::GetParam() EM<=EM_UNKNOWN\n")) return; }
  if (EM==EM_E22_400T30D||EM==EM_E22_400T33D||EM==EM_E22_900T30D) { // Only supported on E22 not E220/32
    if ( EParamGotBytes==0 ) GetParam();                            // Get Parameters
    return paramNETID;
  }
}
//-----------------------------------------------------------------------------------------------------
void TGTEbyte::AirSpeed(byte _Speed) {
  if ( _Speed > 7 ) return;                                     // Check Boundaries

  if (EM<=EM_UNKNOWN) {DBLERROR(("TGTEbyte::GetParam() EM<=EM_UNKNOWN\n")) return; }
  byte CurrentMode=Mode();                                        // Check Radio is Identified
  if (CurrentMode!=ERFSETMODE) Mode(ERFSETMODE);                  // Radio in SETMODE
  if ( EParamGotBytes==0 ) GetParam();                            // Get Parameters if needed.

  paramREG0SPED &= 0xF8;                                          // 0 - previous settings ADR.210
  paramREG0SPED += _Speed;                                        // Set AirSpeed in Parameter

  SetParam();                                                   // Save
  Mode(CurrentMode);                                            // Return to last Mode RF was set to.
}
//-----------------------------------------------------------------------------------------------------
byte TGTEbyte::AirSpeed() {
  if ( EParamGotBytes==0 ) GetParam();                            // Get Parameters
  return (paramREG0SPED&0x07);                                    // ADR.210
}
//-----------------------------------------------------------------------------------------------------
void TGTEbyte::UARTSpeed(byte _SpeedParity) {
  if ( _SpeedParity > 31 ) return;                              // Check Boundaries

  if (EM<=EM_UNKNOWN) {DBLERROR(("TGTEbyte::GetParam() EM<=EM_UNKNOWN\n")) return; }
  byte CurrentMode=Mode();                                        // Check Radio is Identified
  if (CurrentMode!=ERFSETMODE) Mode(ERFSETMODE);                  // Radio in SETMODE
  if ( EParamGotBytes==0 ) GetParam();                            // Get Parameters if needed.

  paramREG0SPED &= 0x07;                 // 0 - previous setting ( bits[76543] )
  paramREG0SPED += (_SpeedParity << 3);  //E32 Parity.76,UARTSPD.543 - E22 UARTSPD.765,Parity.43
  //REG0.765 = UART bps, b000(0)=1200,b001(1)=2400,b010(2)=4800,b011(3)=9600
  //REG0.43 = b00(0)=8N1, b01(1)=8O1, b10(2)=8E1, b11=8N1(3)
  // So for E22 011(UARTSPD.765),00(PAR.43) = 12,0x0C,b1100
  SetParam();                                                   // Save
  Mode(CurrentMode);                                            // Return to last Mode RF was set to.
}
//-----------------------------------------------------------------------------------------------------
byte TGTEbyte::UARTSpeed() {
  if ( EParamGotBytes==0 ) GetParam();                            // Get Parameters
  return ((paramREG0SPED&0xF8)>>3);       //E32 Parity.76,UARTSPD.543 - E22 UARTSPD.765,Parity.43
}
//-----------------------------------------------------------------------------------------------------
// E32 - 'Channel' corresponds to 00H to 1FH and assigns 410 to 441MHz 
// Each (1)Channel is (1)MHz adjust [1F=31].  Default is 17H(23DEC) + 410 = 433.
//-----------------------------------------------------------------------------------------------------
void TGTEbyte::Frequency(int _RFFrequency) {           DBLAENTER((_RFFrequency),("TGTEbyte::Frequency(SET)\n"))
  if (EM==EM_E22_900T30D&&(850>_RFFrequency||_RFFrequency>930)) return;
  if (!EM==EM_E22_900T30D&&(410>_RFFrequency||_RFFrequency>441)) return;  // Check Boundaries

  if (EM<=EM_UNKNOWN) {DBLERROR(("TGTEbyte::GetParam() EM<=EM_UNKNOWN\n")) return; }
  byte CurrentMode=Mode();                                        // Check Radio is Identified
  if (CurrentMode!=ERFSETMODE) Mode(ERFSETMODE);                  // Radio in SETMODE
  if ( EParamGotBytes==0 ) GetParam();                            // Get Parameters if needed.

  if (EM==EM_E22_900T30D) {   paramREG2CHAN=_RFFrequency-850;}        // Set Frequency
  else if (EMS==EMS_E220E22) {paramREG2CHAN=_RFFrequency-410;}
  else if (EMS==EMS_E32) {    paramREG2CHAN=_RFFrequency-410;}

  SetParam();                                                     // Write Parameters
  Mode(CurrentMode);                                              // Return to Current Mode
}
//---------------------------------------------------------------------------------------------------------------------
int TGTEbyte::Frequency() {
  if ( EParamGotBytes==0 ) GetParam();                            // Get Parameters

  if (EM==EM_E22_900T30D) { return Channel()+850;}
  else {                    return Channel()+410;}
}
//-----------------------------------------------------------------------------------------------------
byte TGTEbyte::Channel() {
  if ( EParamGotBytes==0 ) GetParam();                            // Get Parameters
  return paramREG2CHAN;
  //if (EMS==EMS_E32) return (paramCHAN&0x1F);  // Return CHAN.43210
}
//---------------------------------------------------------------------------------------------------------------------
void TGTEbyte::TxPower(byte _TxPower) {
  if ( _TxPower > 3 ) return;                                     // Check Boundaries

  if (EM<=EM_UNKNOWN) {DBLERROR(("TGTEbyte::GetParam() EM<=EM_UNKNOWN\n")) return; }
  byte CurrentMode=Mode();                                        // Check Radio is Identified
  if (CurrentMode!=ERFSETMODE) Mode(ERFSETMODE);                  // Radio in SETMODE
  if ( EParamGotBytes==0 ) GetParam();                            // Get Parameters if needed.

  paramREG1OPT &= 0xFC;                           // 0 - previous setting TX.10
  paramREG1OPT += _TxPower;                       // Set TxPower

  SetParam();                                                     // Write Parameters
  Mode(CurrentMode);                                              // Return to Current Mode
}
//-----------------------------------------------------------------------------------------------------
byte TGTEbyte::TxPower() {
  if ( EParamGotBytes==0 ) GetParam();                              // Get Parameters
  return (paramREG1OPT&0x03);                                       // TX.10
  // Decoded by MenuValue (s) SetNumberName() see RadioTxPower
}
//-----------------------------------------------------------------------------------------------------
void TGTEbyte::FixedTransmission(bool _Fixed) {    DBLAENTER((_Fixed),("TGTEbyte::FixedTransmission[SET](?)\n"))
  if (EM<=EM_UNKNOWN) {DBLERROR(("TGTEbyte::GetParam() EM<=EM_UNKNOWN\n")) return; }
  byte CurrentMode=Mode();                                        // Check Radio is Identified
  if (CurrentMode!=ERFSETMODE) Mode(ERFSETMODE);                  // Radio in SETMODE
  if ( EParamGotBytes==0 ) GetParam();                            // Get Parameters if needed.

  if (EMS==EMS_E220E22) {                                         
    if(_Fixed) {paramREG3 |= 0x40;} else {paramREG3 &= ~0x40;} // E220&E22 REG3.6
  } else if (EMS==EMS_E32) {
    if(_Fixed) {paramREG1OPT |= 0x80;} else {paramREG1OPT &= ~0x80;} // E32 OPT.7
  }

  SetParam();                                                     // Write Parameters
  Mode(CurrentMode);                                              // Return to Current Mode
}
//-----------------------------------------------------------------------------------------------------
bool TGTEbyte::FixedTransmission() {                         DBLENTER(("TGTEbyte::FixedTransmission[GET]()\n"))
  if ( EParamGotBytes==0 ) GetParam();                                    // Get Parameters if needed
  if (EMS==EMS_E220E22)   {return ((paramREG3&0x40)>>6);}                 //E220&E22 REG3.6
  else if (EMS==EMS_E32)  {return ((paramREG1OPT&0x80)>>7);}              //E32 OPT.7
  else {DBLERROR(("TGTEbyte::FixedTransmission[GET]() EMS not set!\n"))}
}
//-----------------------------------------------------------------------------------------------------
void TGTEbyte::E32IODrive(bool _Drive) {
  if (EM<=EM_UNKNOWN) {DBLERROR(("TGTEbyte::GetParam() EM<=EM_UNKNOWN\n")) return; }
  if (EMS==EMS_E220E22) return;                                   // NOT SUPPORTED on E220 or E22
  byte CurrentMode=Mode();                                        // Check Radio is Identified
  if (CurrentMode!=ERFSETMODE) Mode(ERFSETMODE);                  // Radio in SETMODE
  if ( EParamGotBytes==0 ) GetParam();                            // Get Parameters if needed.

  if (EMS==EMS_E32) {
    if (_Drive) {paramREG1OPT+=(1<<6);} else {paramREG1OPT+=(0<<6);}
  }
  SetParam();                                                     // Write Parameters
  Mode(CurrentMode);                                              // Return to Current Mode
}
//-----------------------------------------------------------------------------------------------------
bool TGTEbyte::E32IODrive() {
  if ( EParamGotBytes==0 ) GetParam();                              // Get Parameters
  if (EMS==EMS_E32)     return ((paramREG1OPT&0x40)>>6);            //E32 OPT.6
  // Not supported on E220 & E22
}
//-----------------------------------------------------------------------------------------------------
void TGTEbyte::RSSIOn(bool _On) {
  if (EM<=EM_UNKNOWN) {DBLERROR(("TGTEbyte::GetParam() EM<=EM_UNKNOWN\n")) return; }
  if (EMS==EMS_E32) return;                                       // NOT SUPPORTED on E32
  byte CurrentMode=Mode();                                        // Check Radio is Identified
  if (CurrentMode!=ERFSETMODE) Mode(ERFSETMODE);                  // Radio in SETMODE
  if ( EParamGotBytes==0 ) GetParam();                            // Get Parameters if needed.

  if (EMS==EMS_E220E22) {
    if (_On) {paramREG3 += (1<<7);} else {paramREG3 += (0<<7);}   //E220&E22 REG3.7
  } 
  SetParam();                                                     // Write Parameters
  Mode(CurrentMode);                                              // Return to Current Mode
}
//-----------------------------------------------------------------------------------------------------
bool TGTEbyte::RSSIOn() {
  if ( EParamGotBytes==0 ) GetParam();                              // Get Parameters
  if (EMS==EMS_E220E22) return ((paramREG3&0x80)>>7);    //E220&E22 REG3.7
}
//-----------------------------------------------------------------------------------------------------
void TGTEbyte::LBTOn(bool _On) {
  if (EM<=EM_UNKNOWN) {DBLERROR(("TGTEbyte::GetParam() EM<=EM_UNKNOWN\n")) return; }
  if (EMS==EMS_E32) return;                                       // NOT SUPPORTED on E32
  byte CurrentMode=Mode();                                        // Check Radio is Identified
  if (CurrentMode!=ERFSETMODE) Mode(ERFSETMODE);                  // Radio in SETMODE
  if ( EParamGotBytes==0 ) GetParam();                            // Get Parameters if needed.

  if (EMS==EMS_E220E22) {
    if (_On) {paramREG3 += (1<<4);} else {paramREG3 += (0<<4);}   //E220&E22 REG3.4
  }
  SetParam();                                                     // Write Parameters
  Mode(CurrentMode);                                              // Return to Current Mode
}
//-----------------------------------------------------------------------------------------------------
bool TGTEbyte::LBTOn() {
  if ( EParamGotBytes==0 ) GetParam();                              // Get Parameters
  if (EMS==EMS_E220E22) return ((paramREG3&0x10)>>4);    //E220&E22 REG3.4
}
//-----------------------------------------------------------------------------------------------------
void TGTEbyte::WUORTime(byte _Time) {
  if (EM<=EM_UNKNOWN) {DBLERROR(("TGTEbyte::GetParam() EM<=EM_UNKNOWN\n")) return; }
  byte CurrentMode=Mode();                                        // Check Radio is Identified
  if (CurrentMode!=ERFSETMODE) Mode(ERFSETMODE);                  // Radio in SETMODE
  if ( EParamGotBytes==0 ) GetParam();                            // Get Parameters if needed.

  if (EMS==EMS_E32) {
    paramREG1OPT &= 0xC7;                           // 0 - previous setting E32 WU OPT.543
    paramREG1OPT += (_Time<<3);                     // Set TxPower
  }
  if (EMS==EMS_E220E22) {
    paramREG3 &=0xF8;                                // 0 - previous setting E22 WOR REG3.210
    paramREG3 += _Time;
  }
  SetParam();                                                     // Write Parameters
  Mode(CurrentMode);                                              // Return to Current Mode
}
//-----------------------------------------------------------------------------------------------------
byte TGTEbyte::WUORTime() {
  if ( EParamGotBytes==0 ) GetParam();                              // Get Parameters
  if (EMS==EMS_E32) return ((paramREG1OPT&0x38)>>3);                // E32 WU OPT.543
  if (EMS==EMS_E220E22) return paramREG3&0x03;                      // E22 WOR REG3.210
}
//-----------------------------------------------------------------------------------------------------
void TGTEbyte::E32FECOn(bool _On) {
  if (EM<=EM_UNKNOWN) {DBLERROR(("TGTEbyte::GetParam() EM<=EM_UNKNOWN\n")) return; }
  if (EMS==EMS_E220E22) return;                                   // NOT SUPPORTED on E220&E22
  byte CurrentMode=Mode();                                        // Check Radio is Identified
  if (CurrentMode!=ERFSETMODE) Mode(ERFSETMODE);                  // Radio in SETMODE
  if ( EParamGotBytes==0 ) GetParam();                            // Get Parameters if needed.

  if (EMS==EMS_E32) {
    if (_On) {paramREG1OPT += (1<<2);} else {paramREG1OPT += (0<<2);}  //E32 OPT.2
  }
  SetParam();                                                     // Write Parameters
  Mode(CurrentMode);                                              // Return to Current Mode
}
//-----------------------------------------------------------------------------------------------------
bool TGTEbyte::E32FECOn() {
  if ( EParamGotBytes==0 ) GetParam();                              // Get Parameters
  if (EMS==EMS_E32) return ((paramREG1OPT&0x04)>>2);                // E32 OPT.2
  // NOT SUPPORTED on E220&E22
}
//-----------------------------------------------------------------------------------------------------
void TGTEbyte::E22WOROn(bool _On) {
  if (EM<=EM_UNKNOWN) {DBLERROR(("TGTEbyte::GetParam() EM<=EM_UNKNOWN\n")) return; }
  if (EM==EM_E22_400T30D||EM==EM_E22_400T33D||EM==EM_E22_900T30D) { // Only supported on E22 not E220/32
    byte CurrentMode=Mode();                                        // Check Radio is Identified
    if (CurrentMode!=ERFSETMODE) Mode(ERFSETMODE);                  // Radio in SETMODE
    if ( EParamGotBytes==0 ) GetParam();                            // Get Parameters if needed. 
    if(_On){paramREG3+=(1<<3);}else{paramREG3+=(0<<3);}             // E22 REG3.3
    SetParam();                                                     // Write Parameters
    Mode(CurrentMode);                                              // Return to Current Mode
  }
}
//-----------------------------------------------------------------------------------------------------
bool TGTEbyte::E22WOROn() {
  if ( EParamGotBytes==0 ) GetParam();                              // Get Parameters
  if (EM==EM_E22_400T30D||EM==EM_E22_400T33D||EM==EM_E22_900T30D) return ((paramREG3&0x08)>>3);
}
//---------------------------------------------------------------------------------------------------------------------
void TGTEbyte::SetParam() {                                           DBLENTER(("TGTEbyte::SetParam"))
  // Check Radio is Known and Radio is in SETMODE
  if (EM<=EM_UNKNOWN) {DBLERROR(("TGTEbyte::GetParam() EM<=EM_UNKNOWN\n")) return; }
  byte CurrentMode=Mode(); if (CurrentMode!=ERFSETMODE) Mode(ERFSETMODE);

  // SET Parameters
  if (EMS==EMS_E220E22) {
    SPort.write(0xC0); //Command 0xC0(Write), 0xC1(Read), 0xC2(Temp), 0xFF(Error)
    SPort.write(0x00); //StartReg ( 0x00(00H) to 0x07(07H) )
    if (EM==EM_E220_400T30D) {SPort.write(0x08);} else {SPort.write(0x09);} //Length
    SPort.write(paramADDH);      //00H ADDH
    SPort.write(paramADDL);      //01H ADDL
    if (EM!=EM_E220_400T30D) SPort.write(paramNETID); //02H NETID
    SPort.write(paramREG0SPED);  //03H/02H REG0 UART.765,PARITY.43,AIR.210
    SPort.write(paramREG1OPT);   //04H/03H REG1 SUB.76,RSSI.5,NA.432,TX.01
    SPort.write(paramREG2CHAN);  //05H/04H REG2 FREQ.76543210
    SPort.write(paramREG3);      //06H/05H REG3 RSSI.7,FIX.6,NA.5,LBT.4,NA.3,WOR.210
    SPort.write(paramCRYPTH);    //07H/06H CRYPT_H Key-byte
    SPort.write(paramCRYPTL);    //08H/07H CRYPT_L Key-byte
    
  } else if (EMS==EMS_E32) {
    SPort.write(0xC0);           //b[0]=C0(SAVE) + 5 bytes parameters
    SPort.write(paramADDH);      //b[1]
    SPort.write(paramADDL);      //b[2]
    SPort.write(paramREG0SPED);  //b[3]=SPED   Parity.76,UARTSPD.543,AIRDR.210
    SPort.write(paramREG2CHAN);  //b[4]=CHAN   NA.765,CHAN.43210
    SPort.write(paramREG1OPT);   //b[5]=OPTION FIX.7,IODR.6,WAKE.543,FEC.2,TX.10   
  }
  SPort.flush();
  GetParam(true);       // Reload Parameters after set which replies with new parameters so skip request.
  Mode(CurrentMode);    // Return to last Mode RF was set to.
}
//---------------------------------------------------------------------------------------------------------------------
void TGTEbyte::GetParam(bool _SkipReq) {                                         DBLAENTER((_SkipReq),("TGTEbyte::GetParam"))
  
  // Check Radio is Known and Radio is in SETMODE
  byte CurrentMode=Mode();if (CurrentMode!=ERFSETMODE) Mode(ERFSETMODE);
  if (EM<=EM_UNKNOWN) {DBLERROR(("TGTEbyte::GetParam() EM<=EM_UNKNOWN\n")) return; }
  
  // Request Parameters
  if (!_SkipReq) {
    if (EMS==EMS_E220E22) {     SPort.write(0xC1); //Command 0xC0(Write), 0xC1(Read), 0xC2(Temp), 0xFF(Error)
                                SPort.write(0x00); //Start-Reg
      if (EM==EM_E220_400T30D) {SPort.write(0x08);DBLINFO(("TGTEbyte::GetParam Sending C1 00 08\n"))} 
      else {                    SPort.write(0x09);DBLINFO(("TGTEbyte::GetParam Sending C1 00 09\n"))} //Length

    } else if (EM==EM_E32_433T20D||EM==EM_E32_433T30D) {
                                SPort.write(0xC1);SPort.write(0xC1);SPort.write(0xC1); 
                                DBLINFO(("TGTEbyte::GetParam Sending C1 C1 C1\n"))
    }
  }

  // Parse Reply
  EParamGotBytes=0;bool ReplyOk=true;
  unsigned long StartMillis = millis();
  while ( millis() - StartMillis < 200 ) {
    while ( SPort.available() > 0 ) {
      byte b=SPort.read();DBLAAINFO((EParamGotBytes),(b,HEX),("TGTEbyte::GetParam [?]=?"))
      if (ReplyOk) {
        if (EMS==EMS_E220E22) {
          if (EParamGotBytes<=4) {
            switch(EParamGotBytes) {      //b[0]=Command 0xC0(Write), 0xC1(Read), 0xC2(Temp), 0xFF(Error)
              case 0: if (b!=0xC1) {ReplyOk=false;DBLAERROR((b,HEX),("TGTEbyte::GetParam No 0xC1 reply[0]=\n"))} break;
              case 1: break;              //b[1]=StartReg ( 0x00(00H) to 0x07(07H) )
              case 2: break;              //b[2]=Length ( Number of Parameter-bytes (0-8) )
              case 3: paramADDH=b; break; //b[3]=00H ADDH
              case 4: paramADDL=b; break; //b[4]=01H ADDL
            }
          } else {
            byte idx=0;
            if (EM==EM_E220_400T30D) {idx=EParamGotBytes+1;} else {idx=EParamGotBytes;}
            switch(idx) {
              case 5: paramNETID=b; break;    //b[5]=02H NETID (NOT on E220)
              case 6: paramREG0SPED=b; break; //b[5/6]=03H REG0 UART.765,PARITY.43,AIR.210 (same as E220)
              case 7: paramREG1OPT=b; break;  //b[6/7]=04H REG1 SUB.76,RSSI.5,NA.432,TX.10 (same as E220)
              case 8: paramREG2CHAN=b; break; //b[7/8]=05H REG2 FREQ.76543210 (same as E220)
              case 9: paramREG3=b; break;     //b[8/9]=06H REG3 RSSI.7,FIX.6,REPEATER.5,LBT.4,WOR.3,WOR.210 (Repeater bit)
              case 10:paramCRYPTH=b; break;   //b[9/10]=07H CRYPT_H Key-byte
              case 11:paramCRYPTL=b; break;   //b[10/11]=08H CRYPT_L Key-byte
            }
          }

        } else if (EMS==EMS_E32) {
          switch(EParamGotBytes) {          //b[0]=0xC0
            case 0: if (b!=0xC0) {ReplyOk=false;DBLAERROR((b,HEX),("TGTEbyte::GetParam No 0xC0 reply[0]=\n"))} break;
            case 1: paramADDH=b; break;     //b[1]=ADDH
            case 2: paramADDL=b; break;     //b[2]=ADDL
            case 3: paramREG0SPED=b; break; //b[3]=SPED   Parity.76,UARTSPD.543,AIRDR.210
            case 4: paramREG2CHAN=b; break; //b[4]=CHAN   NA.765,CHAN.43210
            case 5: paramREG1OPT=b; break;  //b[5]=OPTION FIX.7,IODR.6,WAKE.543,FEC.2,TX.10
          }
        }
        if (ReplyOk) EParamGotBytes++;
      } else {DBLAAINFO((EParamGotBytes),(b,HEX),("TGTEbyte::GetParam !ReplyOk[?]=?\n"))}
    }
  }
  DBLAINFO((EParamGotBytes),("TGTEbyte::GetParam EParamGotBytes="))
  if (!_SkipReq) Mode(CurrentMode);    // Return to last Mode RF was set to.
}
//---------------------------------------------------------------------------------------------------------------------
void TGTEbyte::Mode(byte _Mode) {
  if (EM<=EM_UNKNOWN) {DBLERROR(("TGTEbyte::Mode(SET) EM<=EM_UNKNOWN\n")) return; }
  if (EM==EM_E22_400T33D || EM==EM_E22_400T30D || EM==EM_E22_900T30D ) {
    if (      _Mode == ERFNORMMODE) {digitalWrite(PinM1, 0);digitalWrite(PinM0, 0);}
    else if ( _Mode == ERFWAKEMODE) {digitalWrite(PinM1, 0);digitalWrite(PinM0, 1);}
    else if ( _Mode == ERFPWRSAVE ) {digitalWrite(PinM1, 1);digitalWrite(PinM0, 1);}
    else if ( _Mode == ERFSETMODE ) {digitalWrite(PinM1, 1);digitalWrite(PinM0, 0);}
  }
  else if (EM==EM_E32_433T20D || EM==EM_E32_433T30D || EM_E220_400T30D ) {
    if (      _Mode == ERFNORMMODE) {digitalWrite(PinM1, 0);digitalWrite(PinM0, 0);}
    else if ( _Mode == ERFWAKEMODE) {digitalWrite(PinM1, 1);digitalWrite(PinM0, 0);}
    else if ( _Mode == ERFPWRSAVE ) {digitalWrite(PinM1, 0);digitalWrite(PinM0, 1);}
    else if ( _Mode == ERFSETMODE ) {digitalWrite(PinM1, 1);digitalWrite(PinM0, 1);}
  }
  delay(40); // Radio will not respond faster than this.
}
//-----------------------------------------------------------------------------------------------------
byte TGTEbyte::Mode() {
  if (EM<=EM_UNKNOWN) {/*DBLERROR(("TGTEbyte::Mode(GET) EM<=EM_UNKNOWN"))*/ return ERFNORMMODE; }
  int M1=digitalRead(PinM1);int M0=digitalRead(PinM0);
  if (EM==EM_E22_400T33D || EM==EM_E22_400T30D || EM==EM_E22_900T30D ) {
    if (M1==0 && M0==0) return ERFNORMMODE;
    if (M1==0 && M0==1) return ERFWAKEMODE;
    if (M1==1 && M0==1) return ERFPWRSAVE;
    if (M1==1 && M0==0) return ERFSETMODE;
  }
  else if (EM==EM_E32_433T20D || EM==EM_E32_433T30D || EM==EM_E220_400T30D ) {
    if (M1==0 && M0==0) return ERFNORMMODE;
    if (M1==1 && M0==0) return ERFWAKEMODE;
    if (M1==0 && M0==1) return ERFPWRSAVE;
    if (M1==1 && M0==1) return ERFSETMODE;
  }
}
//-----------------------------------------------------------------------------------------------------
void TGTEbyte::CheckRadio() {                                              DBLENTER(("TGTEbyte::CheckRadio"))
  EM=EM_E22_900T30D;EMS=EMS_E220E22;  //version 261A; Static Set check Keypad dead goes away
  if ( EM==EM_NOTGOT ) {
    // Radio Auto-Discovery ForE22
    // REPLY Should have 3HEAD(C1 80 07) +7PINFO[80H-86H] = 10 total reply bytes
    // We use last PInfo-byte as extra-reply garbage bytes
    byte Pinfo[11]={0,0,0,0,0,0,0,0,0,0,0};byte byteIdx=0; 
    DBLINFO(("TGTEbyte::CheckRadio ForE22 SENDING C1 80 07 @ M1=1,M0=0\n"))
    digitalWrite(PinM1, HIGH);digitalWrite(PinM0, LOW);delay(50);
    SPort.write(0xC1);SPort.write(0x80);SPort.write(0x07);SPort.flush();
    unsigned long StartMillis = millis();
    while ( millis() - StartMillis < 200 ) { 
      while ( SPort.available() > 0 ) {
        Pinfo[byteIdx]=byte(SPort.read());
        DBLAAINFO((byteIdx),(Pinfo[byteIdx],HEX),("(idx)=?"))
        if (byteIdx<10) byteIdx++;
      } if (byteIdx==10) break; 
    } DBLAINFO((millis() - StartMillis),("TGTEbyte::CheckRadio ForE22 END-REPLY Duration\n"))

    // Parse Reply
    if ( Pinfo[0]==0xC1 && Pinfo[1]==0x80 && Pinfo[2]==0x07 ) { // E22 3-byte HEAD
      if ( Pinfo[3]==0x00 && Pinfo[4]==0x22 && Pinfo[5]==0x10 && Pinfo[6]==0x1E && Pinfo[7]==0x0B ) {
        EM=EM_E22_900T30D;EMS=EMS_E220E22;
        DBLINFO(("TGTEbyte::CheckRadio ForE22 Discovered EM=EM_E22_900T30D\n"))
      }
      if ( Pinfo[3]==0x00 && Pinfo[4]==0x22 && Pinfo[5]==0x40 && Pinfo[6]==0x21 && Pinfo[7]==0x0A ) {
        EM=EM_E22_400T33D;EMS=EMS_E220E22;
        DBLINFO(("TGTEbyte::CheckRadio ForE22 Discovered EM=EM_E22_400T33D\n"))
      }
    }
  }
  //------------------------------------------------------------------------------------------------------
  if ( EM==EM_NOTGOT ) {
    // Radio Auto-Discovery ForE32
    DBLINFO(("TGTEbyte::CheckRadio ForE32 SENDING C3 C3 C3 @ M1=1,M0=1\n"))
    digitalWrite(PinM1, HIGH);digitalWrite(PinM0, HIGH);delay(50);
    SPort.write(0xC3);SPort.write(0xC3);SPort.write(0xC3);SPort.flush();
    byte Pinfo[5]={0,0,0,0,0};byte byteIdx=0; // PI[0-9]=Product Information bytes
    unsigned long StartMillis = millis();
    while ( millis() - StartMillis < 200 ) {
      while ( SPort.available() > 0 ) {
        Pinfo[byteIdx]=byte(SPort.read());
        DBLAAINFO((byteIdx),(Pinfo[byteIdx],HEX),(" (idx)=?"))
        if (byteIdx<4) byteIdx++;
      } if (byteIdx==4) break; 
    }
    DBLAINFO((millis() - StartMillis),("TGTEbyte::CheckRadio ForE32 END-REPLY Duration\n"))

    // Parse Reply
    if ( Pinfo[0]==0xC3 ) { // E32 1-byte HEAD
      if        ( Pinfo[1]==0x32 ) {  // Is 433MHz
        // 1E is probably equal to a 30D
        if (Pinfo[2]==0x48&&Pinfo[3]==0x14){EM=EM_E32_433T20D;EMS=EMS_E32;DBLINFO(("TGTEbyte::CheckRadio FOUND EM=EM_E32_433T20D\n"))}
        if (Pinfo[2]==0x44&&Pinfo[3]==0x1E){EM=EM_E32_433T30D;EMS=EMS_E32;DBLINFO(("TGTEbyte::CheckRadio FOUND EM=EM_E32_433T30D [E32-TTL-1W]\n"))} //410-441Mhz,YBT17093321
        if (Pinfo[2]==0x49&&Pinfo[3]==0x1E){EM=EM_E32_433T30D;EMS=EMS_E32;DBLINFO(("TGTEbyte::CheckRadio FOUND EM=EM_E32_433T30D [Pre1B](TTL-1W)\n"))} //SN:W020987S00846 Pre-1B
        if (Pinfo[2]==0x51&&Pinfo[3]==0x1E){EM=EM_E32_433T30D;EMS=EMS_E32;DBLINFO(("TGTEbyte::CheckRadio FOUND EM=EM_E32_433T30D [1B]\n"))} //SN:W020583S02336
        if (Pinfo[2]==0x53&&Pinfo[3]==0x1E){EM=EM_E32_433T30D;EMS=EMS_E32;DBLINFO(("TGTEbyte::CheckRadio FOUND EM=EM_E32_433T30D [1B]\n"))} //SN:W020902S07862
        if (Pinfo[2]==0x10&&Pinfo[3]==0x1E){EM=EM_E32_433T30D;EMS=EMS_E32;DBLINFO(("TGTEbyte::CheckRadio FOUND EM=EM_E32_433T30D [V8]\n"))}
      } else if ( Pinfo[1]==0x38 ) {  // Is 470MHz
      } else if ( Pinfo[1]==0x45 ) {  // Is 868MHz
      } else if ( Pinfo[1]==0x44 ) {  // Is 915MHz
      } else if ( Pinfo[1]==0x46 ) {  // Is 170MHz
      } //close[1]
    } //close[0]
  }
  //------------------------------------------------------------------------------------------------------
  if ( EM==EM_NOTGOT ) {
    // Radio Auto-Discovery ForE220
    // Send C1 08 03
    // REPLY Should have 3HEAD(C1 08 03) +3PINFO[08H-0AH] = 6 total reply bytes
    // We use last PInfo-byte as extra-reply garbage bytes
    byte Pinfo[7]={0,0,0,0,0,0,0};byte byteIdx=0; 
    DBLINFO(("TGTEbyte::CheckRadio ForE220 SENDING C1 08 03 @ M1=1,M0=1\n"))
    digitalWrite(PinM1, HIGH);digitalWrite(PinM0, HIGH);delay(50);
    SPort.write(0xC1);SPort.write(0x08);SPort.write(0x03);SPort.flush();
    unsigned long StartMillis = millis();
    while ( millis() - StartMillis < 200 ) { 
      while ( SPort.available() > 0 ) {
        Pinfo[byteIdx]=byte(SPort.read());
        DBLAAINFO((byteIdx),(Pinfo[byteIdx],HEX),("(idx)=?"))
        if (byteIdx<6) byteIdx++;
      } if (byteIdx==6) break; 
    } DBLAINFO((millis() - StartMillis),("TGTEbyte::CheckRadio ForE220 END-REPLY Duration\n"))

    // Parse Reply
    if ( Pinfo[0]==0xC1 && Pinfo[1]==0x08 && Pinfo[2]==0x03 ) { // E220 3-byte HEAD + 3-byte Prod Info
      if ( Pinfo[3]==0x20 && Pinfo[4]==0x0A && Pinfo[5]==0x1E ) {EM=EM_E220_400T30D;EMS=EMS_E220E22;DBLINFO(("TGTEbyte::CheckRadio FOUND EM=EM_E220_400T30D [V1.2]\n"))}
    }
  }
  //------------------------------------------------------------------------------------------------------
  #if defined(FORCERADIOMODEL)
    EM=FORCERADIOMODEL;
  #endif
  if (EM==EM_NOTGOT) {
    EM=EM_UNKNOWN; DBLERROR(("EM=EM_UNKNOWN\n"))
    digitalWrite(PinM1, LOW);digitalWrite(PinM0, LOW);delay(40);
    return; // Exit: No Radio Found.
  } else {
    // FOUND RADIO:  Check Radio Register Settings
    //vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
    GetParam(); 
    DBLAINFO((Frequency()),("TGTEbyte::CheckRadio Radio Freq[mhz]=?\n"))
    DBLAINFO((AirSpeed()),("TGTEbyte::CheckRadio AirSpeed=?\n"))
    DBLAINFO((TxPower()),("TGTEbyte::CheckRadio Radio Power[db]=?\n"))

    // Check Radio-RFAddress matches EEPROM-RFAddress
    unsigned int RFAddress=Address();
    //unsigned int EETode0RFAddress=EEPROM.get(0, EETode0RFAddress); //TodeIndex*AEB_TODEALLOC
    //DBLAINFO((RFAddress,HEX),("TGTEbyte::CheckRadio Radio RFAddress=?\n"))
    //DBLAINFO((EETode0RFAddress,HEX),("TGTEbyte::CheckRadio EEPROM RFAddress=?\n"))
    //if (RFAddress!=EETode0RFAddress) {
      //EEPROM.put(0,RFAddress);
      //DBLINFO(("TGTEbyte::CheckRadio RFAddress!=EETode0RFAddress SET-EEPROM.put(0, RFAddress)\n"))
    //}

    // Check UART ( FOUND EM=EM_E32_433T30D [1B] TGTEbyte::CheckRadio UARTSped=?(3) )
    byte uartspd = UARTSpeed(); // E220&E22 Speed then Parity, E32 Parity then Speed
    DBLAINFO((uartspd),("TGTEbyte::CheckRadio UARTSpeed=?\n"))
    if(EMS==EMS_E220E22&&uartspd!=12) {
      UARTSpeed(12);
      DBLINFO(("TGTEbyte::CheckRadio EMS==EMS_E220E22&&uartspd!=12 SET UARTSpeed(12)\n"))
    }

    bool fixedtrans = FixedTransmission(); // This here GET why is it calling SetParam?
    if (!fixedtrans) {
      DBLINFO(("TGTEbyte::CheckRadio FixedTransmission=false SET FixedTransmission(true)\n"))
      FixedTransmission(true);
    } else {;DBLINFO(("TGTEbyte::CheckRadio FixedTransmission=true"))}

    if (EMS==EMS_E32) { 
      bool FECon = E32FECOn();
      if (!FECon) {
        DBLINFO(("TGTEbyte::CheckRadio FECon=false SET E32FECOn(true)\n"))
        E32FECOn(true);
      } else {;DBLINFO(("TGTEbyte::CheckRadio FECon=true\n"))}
    }
  }
  Mode(ERFNORMMODE);
}

//_____________________________________________________________________________________________________________________