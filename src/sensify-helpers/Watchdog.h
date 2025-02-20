#include "Sensify-config.h"
#include "hardware/watchdog.h"
#include "flash-storage/ObjectCache.h"
#include "mqtt-messages/MQTTMessage.h"
#include <stdio.h>
#include "debug/PrettySerial.h"
#include "sensify-helpers/HealthAlert.h"
#include "network-helpers/MQTTHandler.h"

#define DM_WATCHDOG_NAME "Watchdg"

class Watchdog
{
    public:
        static void Init(uint16_t sensorCoreTimeoutMs, uint16_t mainCoreTimeoutMs);
        static bool Activate();
        static void Poll_SensorCore();
        static void Poll_MainCore();

    private:
    inline static uint16_t _sensorCoreTimeoutMs = 0;
    inline static uint16_t _mainCoreTimeoutMs = 0;
    inline static uint16_t _mainCoreTimer = 0;
    inline static bool _kill = false;

    static void OnTimeout();
};