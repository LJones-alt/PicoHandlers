#include "SensifySelfTest.h"

#define TEST_SWEEP_DELAY 40
#define TEST_IO_SAFETY_DELAY 2

bool SensifySelfTest::Test()
{
    PRINT_HEADING("Running Self-Test");
    IOTest();
    PRINT_SUCCESS(DM_SELFTEST_NAME, "Board test completed successfully");
    return true;
}

void SensifySelfTest::GetError()
{

}

bool SensifySelfTest::IOTest()
{
    //Enable 3v3 reg and wait to stabilise.
    gpio_init(SENSIFY_POWER_IN_3V3_OK);
    gpio_set_pulls(SENSIFY_POWER_IN_3V3_OK, 1, 0 );
    gpio_set_dir(SENSIFY_POWER_IN_3V3_OK, GPIO_IN);
    gpio_init(SENSIFY_POWER_OUT_3V3_ENABLE);
    gpio_set_dir(SENSIFY_POWER_OUT_3V3_ENABLE, GPIO_OUT);
    gpio_put(SENSIFY_POWER_OUT_3V3_ENABLE, true); 
    while(!gpio_get(SENSIFY_POWER_IN_3V3_OK)){            
        sleep_ms(100);
    }     

    ///////// Check IO /////////

    //Sweep on
    for(int i=0; i<SENSIFY_NUM_OF_INPUTS-2; i++)
    {
        // ** WARNING MAGIC SMOKE **
        // HERE BE DRAGONS!!!!
        // NEVER ACTIVATE OUTPUT HIGH WHILE IN OUTPUT MODE ON INPUT PINS
        gpio_init(SENSIFY_IO_IN[i]);
        sleep_ms(TEST_IO_SAFETY_DELAY);
        gpio_put(SENSIFY_IO_IN[i], false); 
        sleep_ms(TEST_IO_SAFETY_DELAY);
        gpio_set_dir(SENSIFY_IO_IN[i], GPIO_OUT);
        gpio_put(SENSIFY_IO_IN[i], false); 
        sleep_ms(TEST_SWEEP_DELAY);
    }
    
    gpio_init(SENSIFY_STATUS_LED_1_GREEN);
    gpio_set_dir(SENSIFY_STATUS_LED_1_GREEN, GPIO_OUT);
    gpio_put(SENSIFY_STATUS_LED_1_GREEN, true); 
    sleep_ms(TEST_SWEEP_DELAY + 2*TEST_IO_SAFETY_DELAY);

    gpio_init(SENSIFY_STATUS_LED_2_GREEN);
    gpio_set_dir(SENSIFY_STATUS_LED_2_GREEN, GPIO_OUT);
    gpio_put(SENSIFY_STATUS_LED_2_GREEN, true); 
    sleep_ms(TEST_SWEEP_DELAY + 2*TEST_IO_SAFETY_DELAY);

    gpio_init(SENSIFY_STATUS_LED_2_ORANGE);
    gpio_set_dir(SENSIFY_STATUS_LED_2_ORANGE, GPIO_OUT);
    gpio_put(SENSIFY_STATUS_LED_2_ORANGE, true); 
    sleep_ms(TEST_SWEEP_DELAY + 2*TEST_IO_SAFETY_DELAY);

    for(int i=SENSIFY_NUM_OF_INPUTS-1; i>=SENSIFY_NUM_OF_INPUTS-2; i--)
    {
        // ** WARNING MAGIC SMOKE **
        // HERE BE DRAGONS!!!!
        // NEVER ACTIVATE OUTPUT HIGH WHILE IN OUTPUT MODE ON INPUT PINS  
        gpio_init(SENSIFY_IO_IN[i]);
        sleep_ms(TEST_IO_SAFETY_DELAY);
        gpio_put(SENSIFY_IO_IN[i], false); 
        sleep_ms(TEST_IO_SAFETY_DELAY);
        gpio_set_dir(SENSIFY_IO_IN[i], GPIO_OUT);
        gpio_put(SENSIFY_IO_IN[i], false); 
        sleep_ms(TEST_SWEEP_DELAY);
    }


    //Check (All inputs should be low)
    for(int i=0; i<SENSIFY_NUM_OF_INPUTS; i++)
    {
        if(gpio_get(SENSIFY_IO_IN[i]))
        {
            PRINT_ERR(DM_SELFTEST_NAME, "Board IO should be low. Fault found on pin: %d", SENSIFY_IO_IN[i]);
            //Uhoh, that's not right. This should be low.
        }            
    }    

    //Sweep off
    for(int i=0; i<SENSIFY_NUM_OF_INPUTS-2; i++)
    {                    
        gpio_set_dir(SENSIFY_IO_IN[i], GPIO_IN);
        sleep_ms(TEST_SWEEP_DELAY + 2*TEST_IO_SAFETY_DELAY);
    }
    
    gpio_put(SENSIFY_STATUS_LED_1_GREEN, false); 
    sleep_ms(TEST_SWEEP_DELAY + 2*TEST_IO_SAFETY_DELAY);
    gpio_put(SENSIFY_STATUS_LED_2_GREEN, false); 
    sleep_ms(TEST_SWEEP_DELAY + 2*TEST_IO_SAFETY_DELAY);
    gpio_put(SENSIFY_STATUS_LED_2_ORANGE, false); 
    sleep_ms(TEST_SWEEP_DELAY + 2*TEST_IO_SAFETY_DELAY);

    for(int i=SENSIFY_NUM_OF_INPUTS-1; i>=SENSIFY_NUM_OF_INPUTS-2; i--)
    {
        gpio_set_dir(SENSIFY_IO_IN[i], GPIO_IN);
        sleep_ms(TEST_SWEEP_DELAY + 2*TEST_IO_SAFETY_DELAY);
    }

    return true;
}