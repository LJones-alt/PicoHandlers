#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/multicore.h"
#include <stdlib.h>
#include "network-helpers/WifiHandler.h"
#include "sensify-sensors/SensorHandler.h" 
#include "network-helpers/MQTTHandler.h"
#include "mqtt-messages/MQTTMessage.h"
#include "time/Time.h"
#include "sensify-outputs/StatusLEDs.h"
#include "sensify-helpers/SensifySelfTest.h"
#include "sensify-helpers/PowerHandler.h"
#include "flash-storage/ObjectCache.h"
#include "mqtt-messages/MQTTSavedMessage.h"
#include "sensify-helpers/PowerHandler.h"
#include "sensify-helpers/Watchdog.h"
#include "debug/PrettySerial.h"
#include "sensify-helpers/HealthAlert.h"
#include "network-helpers/ConnectionSettings.h"
#include "sensify-helpers/SensifyBootSel.h"
#if SENSIFY_ENABLE_OUTPUTS
    #include "sensify-outputs/OutputHandler.h"
#endif
#if SENSIFY_TEST_MODE
    #include "Test.h"
#endif
#if SENSIFY_ENCRYPT_CONNSTR
    #include "encryption/Encryption.h"
#endif

#define DM_MAIN_NAME "Main"

///////////////////////////////////
////////// Key Variables //////////
///////////////////////////////////

//WARNING... If SENSIFY_ENCRYPT_CONNSTR is enabled then you can't set the variables here.
ConnectionSettings::ConnectionData connectionData = 
{
    "CONSTR", //ConnectionString
    {        
        {    //Profile Slot 1
            {
                //Wifi Details
                "", //SSID
                ""  //PW
            },
            {
                //MQTT Details
                "",                             //FQDN
                IPADDR4_INIT_BYTES(0,0,0,0),    //Fallback IP
                8883                            //Port
            },
            {
                //NTP Details
                "",                 //NTP FQDN
                IPADDR4_INIT_BYTES(0,0,0,0)     //NTP Fallback
            },
            true,//Use DHCP
            {
                //Static IP Details (If Required)
                IPADDR4_INIT_BYTES(0,0,0,0),    //IP
                IPADDR4_INIT_BYTES(0,0,0,0),    //Subnet
                IPADDR4_INIT_BYTES(0,0,0,0),    //Gateway
                IPADDR4_INIT_BYTES(0,0,0,0),    //DNS
            }
        }
    }
};

/////////////////////////////////////////////////////////
///// Not-Key (but still need to exist) Variables //////
////////////////////////////////////////////////////////

SensorHandler sensorHandler;
DNSLookup* mqttServers[SENSIFY_NUMBER_OF_WIFI_PROFILES];
FlashStorageController* flashController;
ObjectCache<MQTTSavedMessage::Counts>* savedTelemetryMessages;
ObjectCache<MQTTMessage::Message>* savedHealthMessages;

bool telemetryDataDue = false;
bool healthDataDue = false;
bool ntpSyncDue = false;
bool powerfailureActioned = false;
int messageFailureCount = 0;
err_t status = 0;
bool systemState=true;
bool wifiConnected = true;
size_t telemetryMaxValue = 0;
bool _mqttSendSuccess = false;
uint16_t numberOfHealthMessagesSent = 0;


///////////////////////////////////
/////// Prototype Functions ///////
///////////////////////////////////

void Startup();
void RunCore0();
void onPowerLoss();
void onPowerRestored();
void onSendFailure();
void RunCore1();

bool telemetryTimer_callback(struct repeating_timer *t);
bool healthTimer_callback(struct repeating_timer *t);

bool SendTelemetryMessage(MQTTSavedMessage::Counts *countData, bool reconnectOnFailedSend);
bool SendHealthMessage();