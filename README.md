# EasyNTPClient
Arduino library to read time from Network Time Protocol (NTP) servers.

&nbsp;

## Features
- Handles all heavy lifting involved with managing connections to and parsing time from an NTP server.
- As easy as providing a **UDP** object to the constructor during initialization.
- Works on **Arduino** and **ESP8266**.

&nbsp;

## Example usage
```
/*
 EasyNTPClient example: Basic

 This example shows the basic usage of the EasyNTPClient on a NodeMCU (ESP8266).
 The output is visible in the Serial Monitor at 9600 baud rate.

 For more details see: https://github.com/aharshac/StringSplitter
*/

/* 
* Board/shield platform 
*
* Arduino WiFi shield: #include <WiFi.h>
*
* WiFi 101 shield or MKR1000: #include <WiFi101.h>
*
* ESP8266/NodeMCU: #include <ESP8266WiFi.h>
*
*/
#include <ESP8266WiFi.h>

#include <EasyNTPClient.h>
#include <WiFiUdp.h>


const char *ssid     = "<SSID>";
const char *password = "<PASSWORD>";

WiFiUDP udp;

EasyNTPClient ntpClient(udp, "pool.ntp.org", ((5*60*60)+(30*60))); // IST = GMT + 5:30

void setup(){
  Serial.begin(9600);
  WiFi.begin(ssid, password);

  while ( WiFi.status() != WL_CONNECTED ) {
    delay ( 500 );
    Serial.print ( "." );
  }
}

void loop() {
  Serial.println(ntpClient.getUnixTime());
  
  delay(20000); // wait for 20 seconds before refreshing.
}
```

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