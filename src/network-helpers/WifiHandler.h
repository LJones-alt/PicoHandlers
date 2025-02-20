#ifndef DMWIFIHANDLER_H
#define DMWIFIHANDLER_H


#include <string.h>
#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "lwip/netif.h"
#include "lwip/ip4_addr.h"
#include "Sensify-config.h"
#include "debug/PrettySerial.h"
#include "network-helpers/DNSLookup.h"
#include "network-helpers/NTP.h"
#include "network-helpers/ConnectionSettings.h"

#define DM_WIFIHANDLER_NAME "WiFi"

class WifiHandler{
    public:    
        enum WIFI_STATUS { DISCONNECTED = -2, CONNECTING = -1, CONNECTED =0 };

        static void Init(ConnectionSettings::NetworkProfile wifiSettings[SENSIFY_NUMBER_OF_WIFI_PROFILES], const char* deviceId);
        static bool Connect();
        static bool Connect(int index);

        static bool ReConnect();

        static WIFI_STATUS GetStatus();
        static int GetConnectedProfile();
        static uint8_t GetSignalStrength();
        static const char* GetMacAddress();

    private:
        inline static bool _dhcpEnabled = false;
        inline static ConnectionSettings::NetworkProfile* _wifiSettings = NULL;
        inline static WIFI_STATUS _wifiStatus = DISCONNECTED;
        inline static char _macAddress[18];

        inline static int lastAttemptedProfile = -1;

        inline static const int blockCounterThreshold = 1000;

        static void SetDHCP();
        static void SetStaticIP(ip4_addr* ip, ip4_addr* netmask, ip4_addr* gw, ip4_addr* dns);
};

#endif