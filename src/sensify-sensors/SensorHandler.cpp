#include "SensorHandler.h"


void SensorHandler::Init(){
    PRINT_INFO(DM_SENSORHANDLER_NAME,"Initialising sensors");
    countData=(SensorData::CountData*)calloc(1,sizeof(SensorData::CountData));
    countData->size = SENSIFY_NUM_OF_INPUTS;
    countData->timestamp=Time::GetUnixTimestamp();
    counts = (MQTTSavedMessage::Counts*)calloc(1,sizeof(MQTTSavedMessage::Counts));
    counts->size = SENSIFY_NUM_OF_INPUTS;

    for(int i=0; i<SENSIFY_NUM_OF_INPUTS;i++)
    {
        _risingEdgeCallbacks[i] = NULL;
        _fallingEdgeCallbacks[i] = NULL;
    }

    SetUpSensors();   
    PRINT_SUCCESS(DM_SENSORHANDLER_NAME,"Sensor inputs initialised");     
};

void SensorHandler::Init(MQTTSavedMessage::Counts* initialCount)
{
    Init();
    for(int i=0;i<countData->size;i++)
    {
        countData->sensors[i].count = initialCount->sensors[i].count;
    }
}

bool SensorHandler::Poll(){
    if (PowerHandler::SensorPowerOk())
    {
        PollSensors();
        return true;
    }
    return false;    
};

// return the saved message type to make saving as flash much easier
MQTTSavedMessage::Counts* SensorHandler::GetCountData(){
    counts->timestamp = Time::GetUnixTimestamp();
    for(int i=0;i<countData->size;i++){
        counts->sensors[i].count = countData->sensors[i].count;
        counts->sensors[i].elapsed_on = countData->sensors[i].elapsed_on;
        counts->sensors[i].elapsed_off = countData->sensors[i].elapsed_off;
    }
    PRINT_INFO(DM_SENSORHANDLER_NAME, "Fetched count info ");
    return counts;
};

/// private functions below ///

void SensorHandler::SetUpSensors(){
    for (int i=0; i<SENSIFY_NUM_OF_INPUTS;i++){
        countData->sensors[i].pin_number=SENSIFY_IO_IN[i];
        SetInput(SENSIFY_IO_IN[i]); 
        countData->sensors[i].count=(uint32_t)0;
        countData->sensors[i].state=false;
        countData->sensors[i].elapsed_on=0.0f;
        countData->sensors[i].elapsed_off=0.0f;
    }
};

void SensorHandler::SetInput(int pin){
    gpio_init(pin);
    gpio_set_dir(pin, GPIO_IN);
    PRINT_INFO(DM_SENSORHANDLER_NAME,"Set input pin %d", pin);
}

void SensorHandler::PollSensors(){
    for(int i=0; i<SENSIFY_NUM_OF_INPUTS;i++){
        if(!gpio_get(SENSIFY_IO_IN[i])){ 
            if(!countData->sensors[i].state){
                countData->sensors[i].count++;
                countData->sensors[i].state=true;
                countData->sensors[i].elapsed_off =0; 
                countData->sensors[i].on_timestamp=get_absolute_time();
                countData->sensors[i].elapsed_on=(to_ms_since_boot(get_absolute_time())-to_ms_since_boot(countData->sensors[i].on_timestamp))/1000.0f;
                PRINT_INFO(DM_SENSORHANDLER_NAME,"Core 1: Trigger new on %d, count value now %d", countData->sensors[i].pin_number, (countData->sensors[i].count));
                if(_fallingEdgeCallbacks[i] != NULL)
                {
                    //If callback is not null, call it.
                    _fallingEdgeCallbacks[i]();
                }                
            }
            else{
                countData->sensors[i].elapsed_on=(to_ms_since_boot(get_absolute_time())-to_ms_since_boot(countData->sensors[i].on_timestamp))/1000.0f;
            }
        }else {
            if(countData->sensors[i].state){
                countData->sensors[i].state= false;
                countData->sensors[i].off_timestamp=get_absolute_time();
                countData->sensors[i].elapsed_on=0; 
                if(countData->sensors[i].count>0){
                    // only add offtime if the sensor has ever been used
                    countData->sensors[i].elapsed_off=(to_ms_since_boot(get_absolute_time())-to_ms_since_boot(countData->sensors[i].off_timestamp))/1000.0f;
                }
                PRINT_INFO(DM_SENSORHANDLER_NAME,"Core 1 :GPIO %d rising edge, trigger off.", countData->sensors[i].pin_number );
                if(_risingEdgeCallbacks[i] != NULL)
                {
                    //If callback is not null, call it.
                    _risingEdgeCallbacks[i]();
                }
            }
            else{
                countData->sensors[i].elapsed_off=(to_ms_since_boot(get_absolute_time())-to_ms_since_boot(countData->sensors[i].off_timestamp))/1000.0f;
            }
       }
    }
}

void SensorHandler::SetRisingEdgeCallback(uint inputNumber, SensorHandler::sensor_callback_t cb)
{
    assert(inputNumber < SENSIFY_NUM_OF_INPUTS);
    _risingEdgeCallbacks[inputNumber] = cb;

}

void SensorHandler::SetFallingEdgeCallback(uint inputNumber, SensorHandler::sensor_callback_t cb)
{
    assert(inputNumber < SENSIFY_NUM_OF_INPUTS);
    _fallingEdgeCallbacks[inputNumber] = cb;
}