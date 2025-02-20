#include "SensifyBoard.h"
#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "hardware/gpio.h"
#include <stdio.h>
#include "debug/PrettySerial.h"

#define DM_SELFTEST_NAME "SelfTest"

class SensifySelfTest{
    public:
        static bool Test();
        static void GetError();

    private:
        static bool PwrTest();
        static bool IOTest();
        static bool TimeTest();
};