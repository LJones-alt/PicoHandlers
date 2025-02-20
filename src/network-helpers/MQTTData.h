#ifndef DMMQTTDATA_H
#define DMMQTTDATA_H

#include "lwip/ip4_addr.h"
#include "lwip/apps/mqtt.h"
#include "lwip/altcp_tls.h"
#include "pico/stdlib.h"

class MQTTData{
    public:
        struct MQTTClient{
            mqtt_client_t *client;
            struct mqtt_connect_client_info_t ci;
            const ip_addr_t* ip_addr;
            const ip_addr_t* _temp_ip_addr;
            char* SASToken;
        };
        
};
#endif