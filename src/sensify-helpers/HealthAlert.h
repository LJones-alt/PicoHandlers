#ifndef DMHEALTHALERT_H
#define DMHEALTHALERT_H

#include <stdio.h>
#include "debug/PrettySerial.h"
#include "flash-storage/ObjectCache.h"
#include "mqtt-messages/MQTTMessage.h"
#include "network-helpers/MQTTHandler.h"

#define DM_HEALTHALERT_NAME "HlthAlrt"

class HealthAlert{
    public:
        static void Init(ObjectCache<MQTTMessage::Message>* healthFlashStorage, ObjectCache<MQTTSavedMessage::Counts>* telemetryFlashStorage, char* deviceID);
        static void Submit(const char* message);
        static void Submit(const char* message, bool queueOnly);
        static void SaveMessagesToFlash();

    private:
        inline static ObjectCache<MQTTMessage::Message>* _healthFlashStorage = NULL;
        inline static ObjectCache<MQTTSavedMessage::Counts>* _telemetryFlashStorage = NULL;
        static inline char* _deviceID = NULL ;
};

#endif