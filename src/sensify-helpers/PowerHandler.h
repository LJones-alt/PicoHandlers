#ifndef DMPOWERHANDLER_H
#define DMPOWERHANDLER_H


#include "SensifyBoard.h"
#include "Sensify-config.h"
#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "hardware/gpio.h"
#include <stdio.h>
#include "debug/PrettySerial.h"

#define DM_POWERHANDLER_NAME "Power"

class PowerHandler{
    public:
        static void Init();
        static void Poll();        
        
        static bool SensorPowerOk();        
        static bool PowerFailure();        
        static float GetInputVoltage();
        static bool Is3v3Ok();

    private:
        inline static int cycleSincePowerLoss = 0;
        inline static int cycleSincePowerRestore = 0;
        inline static bool powerHasFailed = true;

        static bool InputPowerPresent();
        static void Set3v3Power(bool enable);        
};

#endif