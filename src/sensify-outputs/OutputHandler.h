#ifndef DMOUTPUTHANDLER_H
#define DMOUTPUTHANDLER_H

#include "SensifyBoard.h"

#include "hardware/gpio.h"
#include "hardware/adc.h"
#include <string.h>
#include <stdio.h>
#include "pico/stdlib.h"
#include "SensifyBoard.h"
#include "Output.h"
#include <algorithm>
#include "debug/PrettySerial.h"

#define DM_OUTPUT_NAME "Outputs"

class OutputHandler{
    public:                
        static void Init();
        static void Poll();
        static OutputData::Outputs* GetOutputs();
    
    private:        
        //3.3v over 4096 steps and current measurement over 2x 27ohm resistors. Then (Multiplied by chip K(ILIS)) to get pin current
        static constexpr float IS_CURRENT_CONVERSION_FACTOR = ((3.3/4096) / 54.0 ); 
        static constexpr float IS_K_ILIS = 2360;
        inline static uint16_t adcOffset = 0;
        inline static bool _initialised = false;

        static void PollReadOutputCurrent();
        static void UpdateOutputStructWithCurrent(OutputData::Output* output, float current);
        static float GetCurrentOfSelectedInput();
        static void Enable(OutputData::Output *output);
        static void Disable(OutputData::Output *output);
        static void SetOutput(OutputData::Output *output, bool state);
        static void CalibrateADC();        
};

#endif