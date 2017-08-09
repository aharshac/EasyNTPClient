# EasyNTPClient 

[![Collaborizm](https://img.shields.io/badge/Collaborizm-Join%20now-blue.svg)](https://www.collaborizm.com/)
[![Build Status](https://travis-ci.org/aharshac/EasyNTPClient.svg?branch=master)](https://travis-ci.org/aharshac/EasyNTPClient)    

Arduino library to read time from Network Time Protocol (NTP) servers.

&nbsp;

## Features
- Handles all heavy lifting involved with managing connections to and parsing time from an NTP server.
- As easy as providing a **UDP** object to the constructor during initialization.
- Works on **Arduino** and **ESP8266**.

&nbsp;

## Examples    
1. **NodeMCU**    
Using EasyNTPClient on a NodeMCU (ESP8266)     

2. **ArduinoEspWifiShield**    
Using EasyNTPClient on an Arduino UNO with an ESP-01 (ESP8266) WiFi module.    
By [**Claran Martis**](https://www.collaborizm.com/profile/SJne7FcMg)

&nbsp;

## Reference
### Class **EasyNTPClient**
#### 1. Initialization ####
1. No frills 
```c
EasyNTPClient(UDP &udp)

Parameters:
    udp: Reference to an UDP object.
Returns:
    EasyNTPClient object.
```

2. Custom server pool 
```c
EasyNTPClient(UDP& udp, const char* serverPool)

Parameters:
    udp: Reference to an UDP object.
	serverPool: NTP server pool. Default = "pool.ntp.org"
Returns:
    EasyNTPClient object.
```

3. Time offset
```c
EasyNTPClient(UDP& udp, const char* serverPool, int offset);

Parameters:
    udp: Reference to an UDP object.
	serverPool: NTP server pool domain name. Default = "pool.ntp.org"
	offset: Difference from UTC in seconds. Default = 0
Returns:
    EasyNTPClient object.
```

#### 2. Methods ###    
1. Get time offset
```c
int getTimeOffset()

Returns:
    EasyNTPClient object.
```

2. Set time offset
```c
void setTimeOffset(int offset);

Parameters:
    offset: Difference from UTC in seconds.
```

3. Get time in UNIX format
```c
unsigned long getUnixTime();

Returns:
    UTC time in UNIX time format (seconds)
```