#ifndef DMMQTTCONNECTIONHELPER_H
#define DMMQTTCONNECTIONHELPER_H
#include <string>
#include <stdio.h>
#include "pico/stdlib.h"
#include "lwip/apps/mqtt.h"
#include "MQTTData.h"
#include "Sensify-config.h"
#include "network-helpers/DNSLookup.h"
#include "mqtt-messages/MQTTMessage.h"
#include "debug/PrettySerial.h"
#include "mbedtls/base64.h"
#include "mbedtls/sha256.h"
#include "network-helpers/ConnectionSettings.h"

#define DM_MQTTCONNECTIONHELPER_NAME "MQTTHlpr"

class MQTTConnectionHelper{
    public:
        char* _connectionString; 
        struct Creds{
                char* Hostname;
                char* DeviceName;
                char* DeviceKey;
                char* MQTTUser;
                char* MQTTTopic;
                char* SASToken;
                long expiry;
                u16_t Port;
            };
        Creds* _creds;
        void Init();
        Creds* GetConnectionSettings(const char* connectionString);
        mqtt_connect_client_info_t* CreateClientInfo();

    private:
        char* SplitOnSymbol(char* str);
        void GetConnectionSettings();
        char* DecodeBase64(char input_str[]);

};

#endif