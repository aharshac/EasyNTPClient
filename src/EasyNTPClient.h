/*
  EasyNTPClient - Arduino library to read time from Network Time Protocol (NTP) servers.
  Created by Harsha Alva, June 29, 2017.
  Released into the public domain.

  Based on work by:
  * Francesco Potortì, 2013
  * https://playground.arduino.cc/Code/NTPclient
  * 
  * Sandeep Mistry, 2016
  * https://github.com/arduino-libraries/NTPClient
*/

#ifndef EasyNTPClient_h
#define EasyNTPClient_h

#include "Arduino.h"
#include <Udp.h>

class EasyNTPClient
{
  public:
    EasyNTPClient(UDP &udp);
    EasyNTPClient(UDP& udp, const char* serverPool);
    EasyNTPClient(UDP& udp, const char* serverPool, int offset);
    int getTimeOffset();
    void setTimeOffset(int offset);
    unsigned long getUnixTime();
    
  private:
    UDP *mUdp;
    const char* mServerPool = "pool.ntp.org";
    int mOffset = 0;
    unsigned int mUpdateInterval = 60000;
    unsigned long mLastUpdate = 0;
    long mServerTime = 0;
    unsigned long getServerTime();
};

#endif