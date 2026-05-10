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
    int udpInited = this->mUdp->begin(NTP_REQUEST_PORT);

    // Only the first four bytes of an NTP request need to be set. The rest
    // must be zero so the server does not interpret them as timestamp offsets.
    byte packetBuffer[NTP_PACKET_SIZE] = {0};
    packetBuffer[0] = NTP_HEADER_LI | NTP_HEADER_VN | NTP_HEADER_MODE;
    // byte 1 (stratum) stays 0 - server ignores client stratum
    packetBuffer[2] = NTP_HEADER_POLL;
    packetBuffer[3] = NTP_HEADER_PRECISION;

    // Fail if WiFiUdp.begin() could not init a socket
    if (!udpInited) {
      this->mUdp->stop();
      return 0;
    }

    // Clear received data from possible stray received packets
    this->mUdp->flush();

    // Send an NTP request
    if (!(this->mUdp->beginPacket(this->mServerPool, NTP_SERVER_PORT)
    && this->mUdp->write(packetBuffer, NTP_PACKET_SIZE) == NTP_PACKET_SIZE
    && this->mUdp->endPacket())) {
      this->mUdp->stop();
      return 0;       // sending request failed
    }

    // Wait for response; check every pollIntv ms up to maxPoll times
    const int pollIntv = 150;   // poll every this many ms
    const byte maxPoll = 15;    // poll up to this many times
    int pktLen;       // received packet length
    for (byte i = 0; i < maxPoll; i++) {
      if ((pktLen = this->mUdp->parsePacket()) == NTP_PACKET_SIZE)
        break;
      delay(pollIntv);
    }
    if (pktLen != NTP_PACKET_SIZE) {
      this->mUdp->stop();
      return 0;       // no correct packet received
    }

    this->mUdp->read(packetBuffer, NTP_PACKET_SIZE);

    // Read the integer part (32 bits) of the Transmit Timestamp at offset 40
    unsigned long time = packetBuffer[NTP_TX_TIMESTAMP_OFFSET];
    time = time << 8 | packetBuffer[NTP_TX_TIMESTAMP_OFFSET + 1];
    time = time << 8 | packetBuffer[NTP_TX_TIMESTAMP_OFFSET + 2];
    time = time << 8 | packetBuffer[NTP_TX_TIMESTAMP_OFFSET + 3];

    // Round to the nearest second if we want accuracy
    // The fractionary part is the next byte divided by 256: if it is
    // greater than 500ms we round to the next second; we also account
    // for an assumed network delay of 50ms, and (0.5-0.05)*256=115;
    // additionally, we account for how much we delayed reading the packet
    // since its arrival, which we assume on average to be pollIntv/2.
    time += (packetBuffer[NTP_TX_TIMESTAMP_OFFSET + 4] > 115 - pollIntv / 8);

    this->mUdp->flush();
    this->mUdp->stop();

    return time - NTP_UNIX_EPOCH_OFFSET;
}

unsigned long EasyNTPClient::getUnixTime() {
  unsigned long delta = millis() - this->mLastUpdate;
  if (this->mServerTime <= 0 || this->mLastUpdate == 0 || delta >= this->mUpdateInterval) {
    unsigned long fetched = this->getServerTime();
    if (fetched > 0) {
      this->mServerTime = fetched;
      this->mLastUpdate = millis();
    }
  }
  return this->mServerTime + ((millis() - this->mLastUpdate) / 1000) + this->mOffset;
}
