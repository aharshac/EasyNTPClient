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

// #pragma once
#include "Arduino.h"
#include "EasyNTPClient.h"

EasyNTPClient::EasyNTPClient (UDP &udp) {
  this->mUdp = &udp;
}

EasyNTPClient::EasyNTPClient (UDP &udp, const char* serverPool) {
  this->mUdp = &udp;
  this->mServerPool = serverPool;
}

EasyNTPClient::EasyNTPClient (UDP &udp, const char* serverPool, int offset) {
  this->mUdp = &udp;
  this->mServerPool = serverPool;
  this->mOffset = offset;
}

int EasyNTPClient::getTimeOffset() {
  return this->mOffset;
}

void EasyNTPClient::setTimeOffset (int offset) {
  this->mOffset = offset;
}


unsigned long EasyNTPClient::getServerTime () {
    static int udpInited = this->mUdp->begin(123); // open socket on arbitrary port
    // Only the first four bytes of an outgoing NTP packet need to be set
    // appropriately, the rest can be whatever.
    const long ntpFirstFourBytes = 0xEC0600E3; // NTP request header

    // Fail if WiFiUdp.begin() could not init a socket
    if (! udpInited)
    return 0;

    // Clear received data from possible stray received packets
    this->mUdp->flush();

    // Send an NTP request
    if (! (this->mUdp->beginPacket(this->mServerPool, 123) // 123 is the NTP port
    && this->mUdp->write((byte *)&ntpFirstFourBytes, 48) == 48
    && this->mUdp->endPacket()))
    return 0;       // sending request failed

    // Wait for response; check every pollIntv ms up to maxPoll times
    const int pollIntv = 150;   // poll every this many ms
    const byte maxPoll = 15;    // poll up to this many times
    int pktLen;       // received packet length
    for (byte i=0; i<maxPoll; i++) {
    if ((pktLen = this->mUdp->parsePacket()) == 48)
      break;
    delay(pollIntv);
    }
    if (pktLen != 48)
    return 0;       // no correct packet received

    // Read and discard the first useless bytes
    // Set useless to 32 for speed; set to 40 for accuracy.
    const byte useless = 40;
    for (byte i = 0; i < useless; ++i)
    this->mUdp->read();

    // Read the integer part of sending time
    unsigned long time = this->mUdp->read();  // NTP time
    for (byte i = 1; i < 4; i++)
    time = time << 8 | this->mUdp->read();

    // Round to the nearest second if we want accuracy
    // The fractionary part is the next byte divided by 256: if it is
    // greater than 500ms we round to the next second; we also account
    // for an assumed network delay of 50ms, and (0.5-0.05)*256=115;
    // additionally, we account for how much we delayed reading the packet
    // since its arrival, which we assume on average to be pollIntv/2.
    time += (this->mUdp->read() > 115 - pollIntv/8);

    // Discard the rest of the packet
    this->mUdp->flush();

    return time + this->mOffset - 2208988800ul;   // convert NTP time to Unix time
}

unsigned long EasyNTPClient::getUnixTime() {
  // if (this->mServerTime < 0) {
  unsigned long delta = millis() - this->mLastUpdate;
  if (this->mServerTime <= 0 || this->mLastUpdate == 0 || delta >= this->mUpdateInterval) {  
    this->mServerTime = this->getServerTime();
    this->mLastUpdate = millis();
  }
  return this->mServerTime + ((millis() - this->mLastUpdate) / 1000);
}
