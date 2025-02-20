#ifndef DMOUTPUT_H
#define DMOUTPUT_H
#include "SensifyBoard.h"

class OutputData{
    public:
        struct Output{
                    int pinNumber;
                    int outputNumber;
                    bool enable;
                    float current;
                    bool fault;            
                };

        struct Outputs{
            Output output[2];
            int numberOfOutputs = 2;
        };

        inline static Outputs outputs = {{
            {SENSIFY_IO_MULTI_1_OUT, 1, false},
            {SENSIFY_IO_MULTI_2_OUT, 2, false}
            }, 2};
};
#endif