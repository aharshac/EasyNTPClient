# EasyNTPClient

Arduino library to read time from Network Time Protocol (NTP) servers.

&nbsp;

## Features

- Handles all heavy lifting involved with managing connections to and parsing time from an NTP server.
- As easy as providing a **UDP** object to the constructor during initialization.
- Works on **Arduino**, **ESP8266**, and **ESP32**.
- Configurable NTP server pool, time offset, and update interval.
- Stale time preservation: returns the last known time when a resync fails.

&nbsp;

## Examples

1. **NodeMCU**  
   Using EasyNTPClient on a NodeMCU (ESP8266).

2. **ArduinoEspWifiShield**  
   Using EasyNTPClient on an Arduino UNO with an ESP-01 (ESP8266) WiFi module.

3. **TestNTPClient**  
   Full test suite covering all features. Runs on ESP8266, ESP32, and Uno+ESP-01.

&nbsp;

## Reference

### Class **EasyNTPClient**

#### 1. Initialization

1. No frills

    ```c
    EasyNTPClient(UDP &udp)

    Parameters:
        udp: Reference to a UDP object.
    Returns:
        EasyNTPClient object.
    ```

1. Custom server pool

    ```c
    EasyNTPClient(UDP& udp, const char* serverPool)

    Parameters:
        udp: Reference to a UDP object.
        serverPool: NTP server pool domain name. Default = "pool.ntp.org"
    Returns:
        EasyNTPClient object.
    ```

1. Time offset

    ```c
    EasyNTPClient(UDP& udp, const char* serverPool, int offset)

    Parameters:
        udp: Reference to a UDP object.
        serverPool: NTP server pool domain name. Default = "pool.ntp.org"
        offset: Difference from UTC in seconds. Default = 0
    Returns:
        EasyNTPClient object.
    ```

1. Time offset and update interval

    ```c
    EasyNTPClient(UDP& udp, const char* serverPool, int offset, uint32_t updateIntervalSecs)

    Parameters:
        udp: Reference to a UDP object.
        serverPool: NTP server pool domain name. Default = "pool.ntp.org"
        offset: Difference from UTC in seconds. Default = 0
        updateIntervalSecs: How often to resync with the NTP server, in seconds. Default = 60
    Returns:
        EasyNTPClient object.
    ```

#### 2. Methods

1. Get Unix time

    ```c
    unsigned long getUnixTime()

    Returns:
        Current time as a Unix timestamp (seconds since 1970-01-01 00:00:00 UTC),
        adjusted by the configured time offset. Resyncs with the NTP server when
        the update interval has elapsed. Returns the last known time if resync fails.
    ```

1. Get time offset

    ```c
    int getTimeOffset()

    Returns:
        Current time offset from UTC in seconds.
    ```

1. Set time offset

    ```c
    void setTimeOffset(int offset)

    Parameters:
        offset: Difference from UTC in seconds.
    ```

1. Get NTP server

    ```c
    const char* getNTPServer()

    Returns:
        The current NTP server pool domain name.
    ```

1. Set NTP server

    ```c
    void setNTPServer(const char* serverPool)

    Parameters:
        serverPool: NTP server pool domain name.
    ```

1. Get update interval

    ```c
    uint32_t getUpdateInterval()

    Returns:
        How often the client resyncs with the NTP server, in seconds.
    ```

1. Set update interval

    ```c
    void setUpdateInterval(uint32_t seconds)

    Parameters:
        seconds: Resync interval in seconds.
    ```

1. Check if last sync succeeded

    ```c
    bool wasUpdated()

    Returns:
        true if the most recent NTP resync attempt succeeded,
        false if it failed or no resync has been attempted yet.
    ```
