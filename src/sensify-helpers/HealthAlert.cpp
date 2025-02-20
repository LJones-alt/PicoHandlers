#include "HealthAlert.h"


void HealthAlert::Init(ObjectCache<MQTTMessage::Message>* healthFlashStorage, ObjectCache<MQTTSavedMessage::Counts>* telemetryFlashStorage, char* deviceID)
{
    _healthFlashStorage = healthFlashStorage;
    _telemetryFlashStorage = telemetryFlashStorage;
    _deviceID = deviceID;
}

void HealthAlert::Submit(const char* message)
{
    HealthAlert::Submit(message, false);
}

void HealthAlert::Submit(const char* message, bool queueOnly)
{
    PRINT_INFO(DM_HEALTHALERT_NAME,"Creating and saving alert: %s", message);
    MQTTMessage::Message* alert_payload = MQTTMessage::CreateHealthMessage(message, _telemetryFlashStorage->Count(), (char*)_deviceID);
    _healthFlashStorage->Add(alert_payload);
    if (queueOnly == false)
    {
        MQTTHandler::PublishMessage(alert_payload);
    }
    
    free(alert_payload);
}

void HealthAlert::SaveMessagesToFlash()
{
    _healthFlashStorage->Save();
    PRINT_SUCCESS(DM_HEALTHALERT_NAME,"Alert messages saved to flash");
}
