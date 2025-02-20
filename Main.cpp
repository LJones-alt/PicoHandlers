#include "Main.h"


void Startup(){

    #if SENSIFY_ENCRYPT_CONNSTR
        Encryption* enc = new Encryption();        
        enc->DecryptMem(reinterpret_cast<uint8_t*>(&connectionData), sizeof(ConnectionSettings::ConnectionData));
        delete enc;
    #endif

    PRINT_HEADING("Power on Self-Test");
    PowerHandler::Init();
    bool powerFailureOnBoot = !PowerHandler::SensorPowerOk();
    bool selfTest = SensifySelfTest::Test();

    //Hands-free programming mode
    if(powerFailureOnBoot && PowerHandler::Is3v3Ok && PowerHandler::SensorPowerOk() == false && PowerHandler::GetInputVoltage() < 2.5)
    {
        //If 24v powerfailure on boot, yet still appears to be running then...
        PRINT_DEBUG_HIGHLIGHT(DM_MAIN_NAME, "Suspected USB only boot..");

        //Switch on Orange LED
        gpio_init(SENSIFY_STATUS_LED_2_ORANGE);
        gpio_set_dir(SENSIFY_STATUS_LED_2_ORANGE, GPIO_OUT);
        gpio_put(SENSIFY_STATUS_LED_2_ORANGE, true);

        sleep_ms(10000);
        if(powerFailureOnBoot && PowerHandler::Is3v3Ok && PowerHandler::SensorPowerOk() == false)
        {
            PRINT_INFO(DM_MAIN_NAME, "Dropping into programming mode");
            REBOOT_SENSIFY_TO_BOOTSEL();
        }
    }

    PRINT_HEADING("Initialise Services");
    StatusLEDs::Init();
    MQTTHandler::Init(connectionData.connectionString);
    WifiHandler::Init(connectionData.networkProfiles, MQTTHandler::GetDeviceID()); 
    Time::Init();
    
    Watchdog::Init(SENSIFY_WATCHDOG_SENSOR_POLL, SENSIFY_WATCHDOG_MAIN_CORE);

    #if SENSIFY_ENABLE_OUTPUTS
        PRINT_INFO(DM_MAIN_NAME, "Enabling 24v output module");
        OutputHandler::Init();
    #endif

    PRINT_HEADING("Initialise MQTT profiles");
    //Init MQTT Handler - will do DNS 
    for(int i = 0; i<SENSIFY_NUMBER_OF_WIFI_PROFILES; i++)
    {
        if(connectionData.networkProfiles[i].mqtt.fallbackIP.addr == IPADDR_NONE)
        {            
            //No fallback IP specified
            mqttServers[i] = new DNSLookup(connectionData.networkProfiles[i].mqtt.serverFQDN);
            PRINT_SUCCESS(DM_MAIN_NAME, "Added MQTT server %s:%u to profile %d without IP fallback", connectionData.networkProfiles[i].mqtt.serverFQDN, connectionData.networkProfiles[i].mqtt.port, i);
        }
        else
        {
            //Fallback ip defined
            mqttServers[i] = new DNSLookup(connectionData.networkProfiles[i].mqtt.serverFQDN, &connectionData.networkProfiles[i].mqtt.fallbackIP,5000);
            PRINT_SUCCESS(DM_MAIN_NAME, "Added MQTT server %s:%u to profile %d with fallback IP: %s", connectionData.networkProfiles[i].mqtt.serverFQDN, connectionData.networkProfiles[i].mqtt.port, i, ipaddr_ntoa(((const ip_addr_t *)&connectionData.networkProfiles[i].mqtt.fallbackIP)));
        }        
    }

    PRINT_HEADING("Initialise Flash Storage");
    flashController = new FlashStorageController(MQTTHandler::GetDeviceID());
    savedTelemetryMessages = new ObjectCache<MQTTSavedMessage::Counts>(250,24,flashController);
    savedHealthMessages = new ObjectCache<MQTTMessage::Message>(10,2,flashController);
    HealthAlert::Init(savedHealthMessages, savedTelemetryMessages, MQTTHandler::GetDeviceID());

    PRINT_HEADING("Initialise Sensors");
    if(savedTelemetryMessages->IsFull())
    {
        //Telemetry currently full. Initilise with previous values.
        PRINT_WARN(DM_MAIN_NAME, "Storage is full, initialising with previous count values");
        MQTTSavedMessage::Counts* lastMessage;
        savedTelemetryMessages->Pop(&lastMessage);
        sensorHandler.Init(lastMessage);
    }
    else
    {
        sensorHandler.Init();
    }
}

int main(void){
    stdio_init_all();    
    adc_init();
    cyw43_arch_init();
    sleep_ms(100); 
    Startup();

    #if SENSIFY_TEST_MODE
        TestApplication(&connectionData);
    #endif

    PRINT_HEADING("Launching Services");
    //Launch Counter Code
    PRINT_INFO(DM_MAIN_NAME, "Starting second core");
    multicore_launch_core1(RunCore1);    
    bool watchDogRecovery = Watchdog::Activate();        
    
    //Connect to WiFi
    PRINT_HEADING("Launching Connectivity Services");
    WifiHandler::Connect();
    int wifiProfile = WifiHandler::GetConnectedProfile();
    Watchdog::Poll_MainCore();

    //Sync Time
    Time::SyncNTP(&connectionData.networkProfiles[wifiProfile].ntp);
    Watchdog::Poll_MainCore();

    //Init MQTT        
    if(wifiProfile >= 0)
    {
        MQTTHandler::UpdateServer(mqttServers[wifiProfile], connectionData.networkProfiles[wifiProfile].mqtt.port);
    }
    
    //Setting up timers
    struct repeating_timer timer;
    bool timer_telemetry = add_repeating_timer_ms(-SENSIFY_SEND_TELEMETRY_INTERVAL, telemetryTimer_callback, NULL, &timer);
    struct repeating_timer healthTimer;
    bool timer_health = add_repeating_timer_ms(-SENSIFY_SEND_HEALTH_INTERVAL, healthTimer_callback, NULL, &healthTimer);       
    
    sleep_ms(1000);
    Watchdog::Poll_MainCore();
    PRINT_INFO(DM_MAIN_NAME, "Queueing power on message");
    // send power ON message    
    if(watchDogRecovery)
    {
        HealthAlert::Submit("Power On from watchdog reset");
    }
    else
    {
        HealthAlert::Submit("Power On");
    }
    //Jump into the main loop    

    RunCore0();
}

void RunCore0()
{
    PRINT_SUCCESS(DM_MAIN_NAME, "Messaging(Core0) loop started");
    MQTTSavedMessage::Counts* recoveredMessage;
    MQTTMessage::Message* recoveredHealthMessage;

    while(1)
    {
        Watchdog::Poll_MainCore();
        sleep_ms(SENSIFY_POLL_RATE * 5 );

        if(PowerHandler::PowerFailure())
        {                        
            //Device running on backup power.            
            if(powerfailureActioned == false)
            { 
                powerfailureActioned = true;
                onPowerLoss();
            }             
        }
        else
        {                
            //Power OK
            if(powerfailureActioned == true)
            {
                powerfailureActioned = false;
                onPowerRestored();
            }

            if(telemetryDataDue)
            {
                telemetryDataDue = false;
                _mqttSendSuccess = SendTelemetryMessage(sensorHandler.GetCountData(), true);
            }

            if(healthDataDue)
            {
                healthDataDue = false;
                Time::SyncExternal();
                SendHealthMessage();
            }
            Watchdog::Poll_MainCore();
            if(ntpSyncDue)
            {
                ntpSyncDue = false;
                long oldTime = Time::GetUnixTimestamp();
                Time::SyncNTP(&connectionData.networkProfiles[WifiHandler::GetConnectedProfile()].ntp);
                long newTime = Time::GetUnixTimestamp();
                Watchdog::Poll_MainCore();

                char message[150];
                int ret = snprintf(message, sizeof(message), 
                "NTP Sync - Timestamp delta: %ld", oldTime-newTime);
                HealthAlert::Submit(message, true);
                PRINT_SUCCESS(DM_MAIN_NAME,"%s", message);
                Watchdog::Poll_MainCore();
            }

            if(_mqttSendSuccess && savedTelemetryMessages->Pop(&recoveredMessage))
            {                
                _mqttSendSuccess = SendTelemetryMessage(recoveredMessage, false);
                Watchdog::Poll_MainCore();
            }

            if(_mqttSendSuccess && savedHealthMessages->Pop(&recoveredHealthMessage))
            {                
                err_t pubState = MQTTHandler::PublishMessage(recoveredHealthMessage);
                Watchdog::Poll_MainCore();
                if(pubState != ERR_OK)
                {
                    savedHealthMessages->Add(recoveredHealthMessage);
                }
            }
        }        
    };
}

void onPowerLoss()
{
    //If the telemetry storage array is full, delete the last value and add a fresh message.
    if(savedTelemetryMessages->IsFull())
    {
        MQTTSavedMessage::Counts* dummyMessage;
        savedTelemetryMessages->Pop(&dummyMessage); //Drop last value
        savedTelemetryMessages->Add(sensorHandler.GetCountData()); //Add current value
    }

    //Save to flash!!!
    savedTelemetryMessages->Save();
    
    //Save "ouch."
    HealthAlert::Submit("Power failure");    
    savedHealthMessages->Save();

    //Send ouch (but leave on flash to be replayed)
    MQTTMessage::Message* powerOffHealthMessage;
    if(savedHealthMessages->Pop(&powerOffHealthMessage))
    {
        savedHealthMessages->Add(powerOffHealthMessage);
        MQTTHandler::PublishMessage(powerOffHealthMessage);    
    }     
}

void onPowerRestored()
{
    HealthAlert::Submit("Power restored");
    PRINT_SUCCESS(DM_MAIN_NAME,"Got power back, getting back to work...");
}

void onSendFailure()
{    
    messageFailureCount++;
    if(messageFailureCount > SENSIFY_MESSAGE_FAIL_THRESHOLD)
    {
        Watchdog::Poll_MainCore();
        PRINT_ERR(DM_MAIN_NAME,"Suspecting connectivity failure. Restarting Wifi");
        PRINT_HEADING("Reconnecting to WiFi and DNS");
        //Several messages have failed to send.

        //Reconnect to the WiFI
        if(WifiHandler::ReConnect())
        {
            //WiFi connected
            Watchdog::Poll_MainCore();

            //Refresh DNS
            int wifiProfile = WifiHandler::GetConnectedProfile();
            if(wifiProfile >= 0)
            {
                MQTTHandler::UpdateServer(mqttServers[wifiProfile], connectionData.networkProfiles[wifiProfile].mqtt.port);
            }
            Watchdog::Poll_MainCore();
            wifiConnected = true;
        }   
        else
        {                          
            //WiFi failed to connect.
            Watchdog::Poll_MainCore();
            if(wifiConnected)
            {
                //If Wifi was connected, but now isn't then save a health alert.
                HealthAlert::Submit("WiFi Disconnected");
            }
            wifiConnected = false;
        }     
         
        messageFailureCount = 0;
    }
}

bool SendTelemetryMessage(MQTTSavedMessage::Counts *countData, bool reconnectOnFailedSend)
{
    PRINT_INFO(DM_MAIN_NAME,"Sending telemetry message");
    MQTTMessage::Message* telemetryMessage = MQTTMessage::CreateTelemetryMessage(countData, MQTTHandler::GetDeviceID());
    status = MQTTHandler::PublishMessage(telemetryMessage);

    if(status!=ERR_OK)
    {
        PRINT_WARN(DM_MAIN_NAME,"Failed to publish, saving message... ");
        if(reconnectOnFailedSend) onSendFailure();
        savedTelemetryMessages->Add(countData);
    }

    telemetryMaxValue = MAX(telemetryMaxValue,savedTelemetryMessages->Count());
    free(telemetryMessage);
    return (status==ERR_OK);
}

bool SendHealthMessage()
{
    PRINT_INFO(DM_MAIN_NAME,"Sending health message");
    bool sentHealthMessage = false;
    MQTTMessage::Message* health_payload = MQTTMessage::CreateHealthMessage("", telemetryMaxValue, MQTTHandler::GetDeviceID());
    status = MQTTHandler::PublishMessage(health_payload);
    if(status!=ERR_OK)
    {
        onSendFailure();
    }
    free(health_payload);
    telemetryMaxValue = 0;
    return (status==ERR_OK);
}


// sensor I/O code running on core 1 - DO NOT TOUCH OR YOU DIE --- Boop. Well, time to die Stefan
void RunCore1()
{
    PRINT_SUCCESS(DM_MAIN_NAME, "Polling(Core1) loop started");
    while(1) {    
        Watchdog::Poll_SensorCore();
        StatusLEDs::Poll(_mqttSendSuccess);
        PowerHandler::Poll();
        sensorHandler.Poll();

        #if SENSIFY_ENABLE_OUTPUTS
            OutputHandler::Poll();
        #endif
        sleep_ms(SENSIFY_POLL_RATE);        
    };  
}

bool telemetryTimer_callback(struct repeating_timer *t){
    telemetryDataDue = true;
    return true;
}

bool healthTimer_callback(struct repeating_timer *t){
    healthDataDue = true;

    numberOfHealthMessagesSent++;
    if((numberOfHealthMessagesSent+1)%SENSIFY_HEALTH_SENDS_PER_NTP_REQUEST == 0)
    {
        ntpSyncDue = true;
    }    
    return true;
}