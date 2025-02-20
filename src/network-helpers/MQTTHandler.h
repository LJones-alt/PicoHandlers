#ifndef DMMQTTHANDLER_H
#define DMMQTTHANDLER_H

#include <string>
#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "lwip/netif.h"
#include "lwip/ip4_addr.h"
#include "lwip/apps/mqtt.h"
#include "lwip/altcp_tls.h"
#include "MQTTData.h"
#include "Sensify-config.h"
#include "network-helpers/DNSLookup.h"
#include "mqtt-messages/MQTTMessage.h"
#include "debug/PrettySerial.h"
#include "mbedtls/base64.h"
#include "mbedtls/sha256.h"
#include "network-helpers/ConnectionSettings.h"
#include "network-helpers/MQTTConnectionHelper.h"

#define DM_MQTTHANDLER_NAME "MQTT"


class MQTTHandler{
    public:
        struct MQTTState{
            bool client_created;
            bool client_connected;
            bool publish_ok;
            bool gotHostame;
            err_t error;
        };
        static void Init(const char* connectionString);
        static err_t PublishMessage(MQTTMessage::Message *message);
        static bool UpdateIP(const ip_addr_t* ip_addr);
        inline static long numMessageSent =0;
        static void UpdateServer(DNSLookup* dns, u16_t port);
        static char* GetDeviceID();

    private:
        static void Disconnect(mqtt_client_t* _client);
        static void CreateClient();
        static err_t ClientConnect(mqtt_client_t* _client);
        static bool CheckIP();
        static void RunTest();
        static void mqtt_connection_cb(mqtt_client_t *_client, void *arg, mqtt_connection_status_t status);
        static void pub_request_cb(void *arg, err_t result);
        static err_t Publish(MQTTMessage::Message* _message);
        inline static MQTTState _mqttState;
        inline static MQTTData::MQTTClient* _mqttData;
        inline static MQTTConnectionHelper connectionHelper;
        inline static MQTTConnectionHelper::Creds* _creds;
        inline static struct mqtt_connect_client_info_t* ci;
        inline static mqtt_client_t* _client;
        inline static bool messageBeingPublished = false;
        inline static bool clientConnecting = false;
        inline static bool clientBusy = false;
        static bool GetIp(DNSLookup* dns);
        
};

#endif