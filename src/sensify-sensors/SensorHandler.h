#ifndef DMSENSORHANDLER_H
#define DMSENSORHANDLER_H

#include "hardware/gpio.h"
#include "hardware/adc.h"
#include <string.h>
#include <stdio.h>
#include "pico/stdlib.h"
#include "SensifyBoard.h"
#include "time/Time.h"
#include "SensorData.h"
#include "mqtt-messages/MQTTSavedMessage.h"
#include "sensify-helpers/PowerHandler.h"
#include "debug/PrettySerial.h"

#define DM_SENSORHANDLER_NAME "Sensors"

#define WAIT_CYCLES 100

class SensorHandler{
    public:
        typedef void (*sensor_callback_t) (void);

        void Init();
        void Init(MQTTSavedMessage::Counts* initialCount);

        bool Poll();
        MQTTSavedMessage::Counts* GetCountData();
        void SetRisingEdgeCallback(uint inputNumber, sensor_callback_t cb);
        void SetFallingEdgeCallback(uint inputNumber, sensor_callback_t cb);
        
    private:
        void SetUpSensors();
        void SetInput(int pin);
        void PollSensors();
        SensorData::CountData* countData;
        MQTTSavedMessage::Counts* counts; // data structure to return to main - gets saved if needed!         

        sensor_callback_t _risingEdgeCallbacks[SENSIFY_NUM_OF_INPUTS];
        sensor_callback_t _fallingEdgeCallbacks[SENSIFY_NUM_OF_INPUTS];
};

#endif