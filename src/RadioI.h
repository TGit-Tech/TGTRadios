/******************************************************************************************************************//**
 * @class   RadioI
 * @brief   Common Interface Base-Class for various RF-Radios
 *********************************************************************************************************************/
#pragma once
#include <Arduino.h>
#include <TGTDB.h>
#ifndef RFSERIAL
  #define RFSERIAL Serial1
#endif
class RadioI {
  public:
    virtual void          Address(unsigned int _RFAddress) {};  ///< SET Radio RF-Address
    virtual unsigned int  Address() {};                         ///< GET Radio RF-Address
    virtual void          AirSpeed(byte _Speed);                ///< SET Radio AirSpeed[3]
    virtual byte          AirSpeed();                           ///< GET Radio AirSpeed[3]
    virtual void          Frequency(int _RFFrequency) {};       ///< SET Radio Frequency Setting
    virtual int           Frequency() {};                       ///< GET Radio Frequency Setting
    virtual void          TxPower(byte _TxPower){};             ///< SET Radio Tx Power Setting
    virtual byte          TxPower(){};                          ///< GET Radio Tx Power Setting

    virtual bool          PacketAvailable(){};
};