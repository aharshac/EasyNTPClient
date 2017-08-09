/*

EasyNTPClient example: NodeMCU

This example shows the basic usage of the EasyNTPClient on a NodeMCU (ESP8266).
The output is visible in the Serial Monitor at 9600 baud rate.

For more details see: https://github.com/aharshac/EasyNTPClient
 
*/

/* 
*
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

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
}

void loop() {
  Serial.println(ntpClient.getUnixTime());
  
  delay(20000); // wait for 20 seconds before refreshing.
}