#include "StatusLEDs.h"

void StatusLEDs::Init()
{
    //LED 1 - Activity
    gpio_init(SENSIFY_STATUS_LED_1_GREEN);
    gpio_set_dir(SENSIFY_STATUS_LED_1_GREEN, GPIO_OUT);
    gpio_put(SENSIFY_STATUS_LED_1_GREEN, false);

    //LED 2 - Status
    gpio_init(SENSIFY_STATUS_LED_2_ORANGE);
    gpio_set_dir(SENSIFY_STATUS_LED_2_ORANGE, GPIO_OUT);
    gpio_put(SENSIFY_STATUS_LED_2_ORANGE, false);

    gpio_init(SENSIFY_STATUS_LED_2_GREEN);
    gpio_set_dir(SENSIFY_STATUS_LED_2_GREEN, GPIO_OUT);
    gpio_put(SENSIFY_STATUS_LED_2_GREEN, false);

    lastPublishMessageCount = 0;
}



void StatusLEDs::Poll(bool mqttSendSuccess)
{
    cycleCount++;

    if(cycleCount % STATUS_LED_CYCLE_SKIP > 0)
    {
        return;
    }    

    //POWER FAILURE
    if(!PowerHandler::SensorPowerOk())
    {
        Poll_Strobe(SENSIFY_STATUS_LED_2_ORANGE);
        Poll_Off(SENSIFY_STATUS_LED_2_GREEN);
        Poll_Off(SENSIFY_STATUS_LED_1_GREEN);
        return;
    }

    //WIFI CONNECTING
    if(WifiHandler::GetStatus() == WifiHandler::WIFI_STATUS::CONNECTING)
    {
        Poll_Steady(SENSIFY_STATUS_LED_2_ORANGE);
        Poll_FastFlash(SENSIFY_STATUS_LED_2_GREEN);
        Poll_Off(SENSIFY_STATUS_LED_1_GREEN);
        return;
    }

    //WIFI FAILURE
    if(WifiHandler::GetStatus() == WifiHandler::WIFI_STATUS::DISCONNECTED)
    {
        Poll_FastFlash(SENSIFY_STATUS_LED_2_ORANGE);
        Poll_Off(SENSIFY_STATUS_LED_2_GREEN);
        Poll_Off(SENSIFY_STATUS_LED_1_GREEN);
        return;
    }

    //MQTT FAILURE
    if(mqttSendSuccess == false)
    {
        Poll_SlowFlash(SENSIFY_STATUS_LED_2_GREEN);
        Poll_Off(SENSIFY_STATUS_LED_2_ORANGE);
        Poll_Off(SENSIFY_STATUS_LED_1_GREEN);
        return;
    }


   //All is well!
   Poll_Steady(SENSIFY_STATUS_LED_2_GREEN);
   Poll_Off(SENSIFY_STATUS_LED_2_ORANGE);

    if(lastPublishMessageCount != MQTTHandler::numMessageSent)
    {
        //Toggle activity light on message count
        lastPublishMessageCount = MQTTHandler::numMessageSent;
        LED_TOGGLE(SENSIFY_STATUS_LED_1_GREEN);
    }    

}

void StatusLEDs::Poll_Steady(const int pin)
{
    LED_ON(pin);
}

void StatusLEDs::Poll_Off(const int pin)
{
    LED_OFF(pin);
}

void StatusLEDs::Poll_FastFlash(const int pin)
{
    if(((cycleCount / STATUS_LED_CYCLE_SKIP) % STATUS_LED_FAST_FLASH_MOD) == 0)
    {
        LED_TOGGLE(pin);
    }
}

void StatusLEDs::Poll_SlowFlash(const int pin)
{
    if(((cycleCount / STATUS_LED_CYCLE_SKIP) % STATUS_LED_SLOW_FLASH_MOD) == 0)
    {
        LED_TOGGLE(pin);
    }
}

void StatusLEDs::Poll_Strobe(const int pin)
{
    if(((cycleCount / STATUS_LED_CYCLE_SKIP) % STATUS_LED_SLOW_FLASH_MOD) == 0)
    {
        LED_ON(pin);
    }
    else
    {
        LED_OFF(pin);
    }
}
