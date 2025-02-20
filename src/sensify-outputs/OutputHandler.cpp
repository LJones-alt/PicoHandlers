#include "OutputHandler.h"

void OutputHandler::Init()
{
    for(int i=0; i<OutputData::outputs.numberOfOutputs; i++)
    {
        int pinNumber = OutputData::outputs.output[i].pinNumber;
        gpio_init(OutputData::outputs.output[i].pinNumber);
        gpio_set_dir(OutputData::outputs.output[i].pinNumber, GPIO_OUT);
        gpio_put(OutputData::outputs.output[i].pinNumber, false); 
    }

    adcOffset = 0;
    adc_init();
    adc_gpio_init(SENSIFY_OUTIC_INANALOG_IS_PIN);    

    gpio_init(SENSIFY_OUTIC_OUT_DEN);
    gpio_set_dir(SENSIFY_OUTIC_OUT_DEN, GPIO_OUT);
    gpio_put(SENSIFY_OUTIC_OUT_DEN, true);

    gpio_init(SENSIFY_OUTIC_OUT_DSEL);
    gpio_set_dir(SENSIFY_OUTIC_OUT_DSEL, GPIO_OUT);
    gpio_put(SENSIFY_OUTIC_OUT_DSEL, true);

    CalibrateADC();

    _initialised = true;
}


void OutputHandler::Poll()
{    
    PollReadOutputCurrent();    
    
    for(int i=0; i<OutputData::outputs.numberOfOutputs; i++)
    {        
        gpio_put(OutputData::outputs.output[i].pinNumber, OutputData::outputs.output[i].enable); 
    }
}


void OutputHandler::PollReadOutputCurrent()
{
    //Read current ADC, then set up IO to read next output
    float current = GetCurrentOfSelectedInput();

    if(gpio_get_out_level(SENSIFY_OUTIC_OUT_DSEL))
    {
        //True, so input 2
        UpdateOutputStructWithCurrent(&OutputData::outputs.output[1], current);
        gpio_put(SENSIFY_OUTIC_OUT_DSEL, false); //Flip output for next cycle
    }
    else
    {        
        //False, so input 1        
        UpdateOutputStructWithCurrent(&OutputData::outputs.output[0], current);
        gpio_put(SENSIFY_OUTIC_OUT_DSEL, true); //Flip output for next cycle
    }    
}

void OutputHandler::UpdateOutputStructWithCurrent(OutputData::Output* output, float current)
{
    if(output->enable)
    {
        if(current > 0)
        {
            //current positive, all good.
            output->current = current;
            output->fault = false;
        }
        else
        {
            //current negative, short detected
            output->current = 0;
            output->fault = true;
        }
    }
    else
    {
        //output off, so zero for now
        output->current = current;
        output->fault = false;
    }
}

float OutputHandler::GetCurrentOfSelectedInput()
{
    adc_select_input(SENSIFY_OUTIC_INANALOG_IS_ADC);

    uint16_t adcRead = adc_read();
    float pinCurrent = 0;

    if(adcRead > adcOffset)
    {
        pinCurrent = (adcRead - adcOffset) * IS_CURRENT_CONVERSION_FACTOR;
    }

    if(pinCurrent > SENSIFY_OUTIC_IS_FAULT_CURRENT_THRESHOLD)    
    {
        //Fault
        return -1;
    }
    else
    {
        //Convert to output current using k(ILIS)
        return pinCurrent * IS_K_ILIS;
    }
}

void OutputHandler::Enable(OutputData::Output *output)
{
    SetOutput(output, true);
}

void OutputHandler::Disable(OutputData::Output *output)
{
    SetOutput(output, false);
}

void OutputHandler::SetOutput(OutputData::Output *output, bool state)
{
    gpio_put(output->pinNumber, state); 
}

void OutputHandler::CalibrateADC()
{       
    //Switch off current output, and point the ADC at the port 
    gpio_put(SENSIFY_OUTIC_OUT_DEN, true);
    adc_select_input(SENSIFY_OUTIC_INANALOG_IS_ADC);

    //Wait to stabilise
    sleep_ms(100);
    adcOffset = 300;
    for(int i = 0; i < 10; i++)
    {        
        sleep_ms(1);
        adcOffset =  std::min(adcOffset, adc_read());        
    }

    gpio_put(SENSIFY_OUTIC_OUT_DEN, true);
    sleep_ms(5);
    PRINT_INFO(DM_OUTPUT_NAME,"Offsetting ADC by: %u", adcOffset);    
}

OutputData::Outputs* OutputHandler::GetOutputs()
{
    assert(_initialised);
    return &OutputData::outputs;
}