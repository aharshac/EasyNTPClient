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

#define NTP_PACKET_SIZE         48          // total NTP packet size in bytes
#define NTP_TX_TIMESTAMP_OFFSET 40          // byte offset of Transmit Timestamp field
#define NTP_SERVER_PORT         123         // well-known NTP UDP port
#define NTP_REQUEST_PORT        1123        // local UDP socket port

#define NTP_HEADER_LI           0b11000000  // Leap Indicator = 3 (unsynchronized)
#define NTP_HEADER_VN           0b00100000  // Version Number = 4
#define NTP_HEADER_MODE         0b00000011  // Mode = 3 (client)
#define NTP_HEADER_POLL         6           // poll interval as log2 seconds (2^6 = 64 s)
#define NTP_HEADER_PRECISION    0xEC        // system clock precision in log2 seconds

// Seconds between the NTP epoch (1900-01-01) and the Unix epoch (1970-01-01).
// NTP timestamps are based on 1900; Unix timestamps on 1970. Subtracting this
// constant converts a raw NTP timestamp to a Unix timestamp.
#define NTP_UNIX_EPOCH_OFFSET   2208988800ul

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
    uint32_t mUpdateInterval = 60000;
    unsigned long mLastUpdate = 0;
    long mServerTime = 0;
    unsigned long getServerTime();
};

#endif
