#ifndef DMSENSORDATA_H
#define DMSENSORDATA_H

#include "time/Time.h"
#include "pico/stdlib.h"

class SensorData{
    public:
        struct Sensor{
            uint32_t count=0;
            int pin_number=0;
            bool state=false;
            absolute_time_t on_timestamp;
            float elapsed_on=0.0;
            absolute_time_t off_timestamp;
            float elapsed_off=0.0;
        };
        struct CountData{
            Sensor sensors[SENSIFY_NUM_OF_INPUTS];
            int size = SENSIFY_NUM_OF_INPUTS;  
            long timestamp;
        };
};
#endif