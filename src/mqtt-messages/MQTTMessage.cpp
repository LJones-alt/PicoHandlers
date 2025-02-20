#include "MQTTMessage.h"

void MQTTMessage::Init(){
};

MQTTMessage::Message* MQTTMessage::CreateTelemetryMessage(MQTTSavedMessage::Counts* data, char* deviceID){
    Message* message = (Message*)calloc(1, sizeof(Message));
    char* str = message->payload;
    int ret = snprintf(str, sizeof(message->payload),  "{\"id\": \"%s\", \"timestamp\": %ld, \"type\": \"telemetry\"", deviceID, data->timestamp);
    for (int i = 0; i<10; i++)
    {
        ret = snprintf(str, sizeof(message->payload), "%s,\"input_%d\": {\"value\":%ld, \"on_time\": %f, \"off_time\":%f}", str, i+1, data->sensors[i].count, data->sensors[i].elapsed_on, data->sensors[i].elapsed_off);
    }
    ret = snprintf(str, sizeof(message->payload), "%s}", str);
    PRINT_INFO(DM_MESSAGEHANDLER_NAME, "Made telemetry message, timestamp: %ld", data->timestamp);
    return message;
};



MQTTMessage::Message* MQTTMessage::CreateHealthMessage(const char* statusMessage, size_t messageBufferSize, char* deviceID)
{   
    long timestamp = Time::GetUnixTimestamp();
    uint8_t signalstrength = WifiHandler::GetSignalStrength();
    float powerstate = PowerHandler::GetInputVoltage();

    HealthData* healthData = (HealthData*)calloc(1, sizeof(HealthData));
    healthData->timestamp = Time::GetUnixTimestamp();
    healthData->statusMessage = statusMessage;
    healthData->messageBufferSize = messageBufferSize;
    healthData->temp = Time::GetTemp();
    healthData->wifiprofile = WifiHandler::GetConnectedProfile();
    healthData->wifiStrength = WifiHandler::GetSignalStrength();
    #if SENSIFY_ENABLE_OUTPUTS
    healthData->outCurrent1 = OutputHandler::GetOutputs()->output[0].current;
    healthData->outCurrent2 = OutputHandler::GetOutputs()->output[1].current;
    #else
    healthData->outCurrent1 = 0.0f;
    healthData->outCurrent2 = 0.0f;
    #endif
    healthData->primaryPower = PowerHandler::SensorPowerOk();
    healthData->voltage24v = PowerHandler::GetInputVoltage();
    healthData->sensorLogicPower = PowerHandler::Is3v3Ok();
    healthData->mac = WifiHandler::GetMacAddress();
   // healthData->firwareversion = FIRMWARE_VERSION;
    MQTTMessage::Message* outputMessage = CreateHealth(healthData, deviceID);
    PRINT_INFO(DM_MESSAGEHANDLER_NAME, "Made health message, timestamp: %ld", healthData->timestamp);
    free(healthData);
    
    return outputMessage;
};


MQTTMessage::Message* MQTTMessage::CreateHealth(HealthData* healthData, char* deviceID)
{
    Message* message = (Message*)calloc(1, sizeof(Message));
    char* str = message->payload;
    int ret = snprintf(str, sizeof(message->payload), 
    "{"
    "\"type\": \"health\","
    "\"id\": \"%s\","    
    "\"firmware\": \"%s\","
    "\"timestamp\": %ld,"    
    "\"message\": \"%s\","
    "\"messageBuffer\": \"%zu\","
    "\"wifiStrength\": %u,"
    "\"wifiProfile\": %d,"
    "\"outCurrent1\": %f,"
    "\"outCurrent2\": %f,"
    "\"primaryPower\": \"%s\","
    "\"primaryVoltage\": %f,"
    "\"sensorLogicPower\": \"%s\","
    "\"temp\": %u,"
    "\"mac\": \"%s\""
    "}",
    deviceID,
    FIRMWARE_VERSION,
    healthData->timestamp,
    healthData->statusMessage,
    healthData->messageBufferSize,
    healthData->wifiStrength,
    healthData->wifiprofile,
    healthData->outCurrent1,
    healthData->outCurrent2,
    healthData->primaryPower ? "true" : "false",
    healthData->voltage24v,
    healthData->sensorLogicPower ? "true" : "false",
    healthData->temp,
    healthData->mac);

    return message;
};


