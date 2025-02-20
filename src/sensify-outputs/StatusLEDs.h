#include "hardware/gpio.h"
#include <stdio.h>
#include "pico/stdlib.h"
#include "SensifyBoard.h"
#include "network-helpers/MQTTHandler.h"
#include "network-helpers/WifiHandler.h"
#include "sensify-sensors/SensorHandler.h"
#include "sensify-helpers/PowerHandler.h"

#define STATUS_LED_CYCLE_SKIP 10
#define STATUS_LED_FAST_FLASH_MOD 2
#define STATUS_LED_SLOW_FLASH_MOD 6

#define LED_TOGGLE(pin) gpio_put(pin, !gpio_get_out_level(pin));
#define LED_OFF(pin) gpio_put(pin, false);
#define LED_ON(pin) gpio_put(pin, true);

class StatusLEDs{
    public:                
        static void Init();
        static void Poll(bool mqttSendSuccess);
    private:
        inline static long lastPublishMessageCount = 0;
        inline static uint cycleCount = 0;

        static void Poll_Steady(const int pin);
        static void Poll_FastFlash(const int pin);
        static void Poll_SlowFlash(const int pin);
        static void Poll_Strobe(const int pin);
        static void Poll_Off(const int pin);
};