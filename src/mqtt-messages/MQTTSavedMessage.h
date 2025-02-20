#ifndef DMSAVEDMESSAGE_H
#define DMSAVEDMESSAGE_H

#include "SensifyBoard.h"

class MQTTSavedMessage{
    public:
        struct Sensor{
            uint32_t count=0;
            float elapsed_on=0.0;
            float elapsed_off=0.0;
        };
        struct Counts{
            Sensor sensors[SENSIFY_NUM_OF_INPUTS];
            int size = SENSIFY_NUM_OF_INPUTS; 
            long timestamp;
        };
};
#endif