#include "PowerHandler.h"

void PowerHandler::Init()
{

    //3.3 volts
    gpio_init(SENSIFY_POWER_IN_3V3_OK);
    gpio_set_pulls(SENSIFY_POWER_IN_3V3_OK, 1, 0 );
    gpio_set_dir(SENSIFY_POWER_IN_3V3_OK, GPIO_IN);

    gpio_init(SENSIFY_POWER_OUT_3V3_ENABLE);
    gpio_set_dir(SENSIFY_POWER_OUT_3V3_ENABLE, GPIO_OUT);
    gpio_put(SENSIFY_POWER_OUT_3V3_ENABLE, false); 

    //24 volts
    gpio_init(SENSIFY_POWER_IN_24V_NOT_OK);
    gpio_set_dir(SENSIFY_POWER_IN_24V_NOT_OK, GPIO_IN);

    gpio_init(SENSIFY_POWER_INANALOG_24V_PIN);
    gpio_set_dir(SENSIFY_POWER_INANALOG_24V_PIN, GPIO_IN);
    adc_gpio_init(SENSIFY_POWER_INANALOG_24V_PIN);

    sleep_ms(10);
    powerHasFailed = !InputPowerPresent();
}

void PowerHandler::Poll()
{
    if(InputPowerPresent())
    {        
        //Power Good!
        if(powerHasFailed)
        {            
            if(cycleSincePowerRestore++ > SENSIFY_POWER_RESTORE_THRESHOLD)
            {
                //Power has restored and stabilised.
                cycleSincePowerLoss = 0;
                powerHasFailed=false;
                Set3v3Power(true);
                PRINT_SUCCESS(DM_POWERHANDLER_NAME,"Power restored and stabilised");
            }
            else
            {
                return;
            }
        }
    }
    else
    {
        //Power BAD!
        if(!powerHasFailed)
        {            
            cycleSincePowerRestore = 0;
            if(cycleSincePowerLoss++ > SENSIFY_POWER_RESTORE_THRESHOLD)
            {
                //Power has failed and timed out.
                cycleSincePowerRestore = 0;
                powerHasFailed=true;
                Set3v3Power(false);
                PRINT_ERR(DM_POWERHANDLER_NAME,"24v Power failure.");
            }
            else
            {
                return;
            }
        }
    }
}

bool PowerHandler::SensorPowerOk()
{
    return InputPowerPresent() && Is3v3Ok();
   // return true;
}

bool PowerHandler::PowerFailure()
{
    return powerHasFailed;
   // return false;
}

float PowerHandler::GetInputVoltage()
{
    adc_select_input(SENSIFY_POWER_INANALOG_24V_ADC);
    sleep_ms(1);
    return adc_read() * SENSIFY_CONST_ANALOG_24V_MULTIPLIER;
}

bool PowerHandler::InputPowerPresent()
{
    return !gpio_get(SENSIFY_POWER_IN_24V_NOT_OK);
}

void PowerHandler::Set3v3Power(bool enable)
{
    gpio_put(SENSIFY_POWER_OUT_3V3_ENABLE, enable); 
}

bool PowerHandler::Is3v3Ok()
{
    return gpio_get(SENSIFY_POWER_IN_3V3_OK);
}