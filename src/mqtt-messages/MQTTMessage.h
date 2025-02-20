#ifndef DMMQTTMESSAGE_H
#define DMMQTTMESSAGE_H
#include <string.h>
#include <stdio.h>
#include "sensify-sensors/SensorData.h" 
#include "pico/stdlib.h"
#include "Sensify-config.h"
#include "MQTTSavedMessage.h"
#include "time/Time.h"
#include "network-helpers/WifiHandler.h"
#include "sensify-helpers/PowerHandler.h"
#if SENSIFY_ENABLE_OUTPUTS
    #include "sensify-outputs/OutputHandler.h"
#endif

#define DM_MESSAGEHANDLER_NAME "Message"

class MQTTMessage{
    public:
        static void Init();
        struct Message{
            char payload[SENSIFY_MQTT_MESSAGE_SIZE];
            err_t status;
        };

        static Message* CreateTelemetryMessage(MQTTSavedMessage::Counts* data, char* deviceID);
        static Message* CreateHealthMessage(const char* statusMessage, size_t messageBufferSize, char* deviceID);        
        
    private:
        struct HealthData{
            long timestamp;
            const char* statusMessage;
            size_t messageBufferSize;
            uint8_t temp;
            int wifiprofile;
            uint8_t wifiStrength;
            float outCurrent1;
            float outCurrent2;
            bool primaryPower;
            float voltage24v;
            bool sensorLogicPower;
            const char* mac;  
            const char* firwareversion;
        };

        static Message* _message;
        static Message* _healthMessage;
        static Message* CreateTelemetryMessage(MQTTSavedMessage::Counts* data, long timestamp, Message* _message, char* deviceID);
        static void FormatAlertMessage();
        static Message* CreateHealth(HealthData* healthData, char* deviceID);
};
#endif