#include "Test.h"

void TestApplication(ConnectionSettings::ConnectionData *connectionData){
    PRINT_DEBUG_HIGHLIGHT(DM_DEBUG_NAME, "********************** ENTERING DEBUG MODE **************************");
    PRINT_ERR(DM_DEBUG_NAME,             "********************** ENTERING DEBUG MODE **************************");
    PRINT_DEBUG_HIGHLIGHT(DM_DEBUG_NAME, "********************** ENTERING DEBUG MODE **************************");
    PRINT_HEADING("Debug Mode");

    PrintConnectionStringMemoryAllocation(connectionData);

    //IP_ADDR test
    PRINT_HEADING("IP Addr storage test");
    ip_addr_t test = IPADDR4_INIT_BYTES(1,2,3,4);
    PrintBytesOfObject("test",(char *) &test, sizeof(test));

    ip_addr_t test2 = IPADDR4_INIT_BYTES(0xAF,0xBF,0xCF,0xDF);
    PrintBytesOfObject("test2",(char *) &test2, sizeof(test2));

    ip_addr_t test3 = IPADDR4_INIT_BYTES(192,168,0,1);
    PrintBytesOfObject("test3",(char *) &test3, sizeof(test3));

    while(1);
}

void PrintConnectionStringMemoryAllocation(ConnectionSettings::ConnectionData *connectionData)
{       
    char* offset = (char *) connectionData;

    PRINT_HEADING("ConnectionString Dimensions");
    PRINT_INFO(DM_DEBUG_NAME, "SizeOf: %d bytes", sizeof(*connectionData));    
    PRINT_VARIABLE(connectionData->connectionString, offset);    
    PRINT_VERBOSE(DM_DEBUG_NAME, "%s", connectionData->connectionString);

    PRINT_VARIABLE(connectionData->networkProfiles, offset);
    for(int i=0; i<SENSIFY_NUMBER_OF_WIFI_PROFILES; i++)
    {
        PRINT_VARIABLE(connectionData->networkProfiles[i], offset);        
        //Wifi
        PRINT_VARIABLE(connectionData->networkProfiles[i].wifi.ssid, offset);
        PRINT_VERBOSE(DM_DEBUG_NAME, "%s", connectionData->networkProfiles[i].wifi.ssid);
        PRINT_VARIABLE(connectionData->networkProfiles[i].wifi.pass, offset);
        PRINT_VERBOSE(DM_DEBUG_NAME, "%s", connectionData->networkProfiles[i].wifi.pass);

        //MQTT
        PRINT_VARIABLE(connectionData->networkProfiles[i].mqtt.serverFQDN, offset);
        PRINT_VERBOSE(DM_DEBUG_NAME, "%s", connectionData->networkProfiles[i].mqtt.serverFQDN);
        PRINT_VARIABLE(connectionData->networkProfiles[i].mqtt.fallbackIP, offset);
        PRINT_VERBOSE(DM_DEBUG_NAME, "%s", ipaddr_ntoa(((const ip_addr_t *)&connectionData->networkProfiles[i].mqtt.fallbackIP)));        
        PRINT_VARIABLE(connectionData->networkProfiles[i].mqtt.port, offset);
        PRINT_VERBOSE(DM_DEBUG_NAME, "%d", connectionData->networkProfiles[i].mqtt.port);

        //NTP
        PRINT_VARIABLE(connectionData->networkProfiles[i].ntp.ntpFQDN, offset);
        PRINT_VERBOSE(DM_DEBUG_NAME, "%s", connectionData->networkProfiles[i].ntp.ntpFQDN);
        PRINT_VARIABLE(connectionData->networkProfiles[i].ntp.fallbackIP, offset);        
        PRINT_VERBOSE(DM_DEBUG_NAME, "%s", ipaddr_ntoa(((const ip_addr_t *)&connectionData->networkProfiles[i].ntp.fallbackIP)));        

        //Static IP
        PRINT_VARIABLE(connectionData->networkProfiles[i].useDHCP, offset);
        PRINT_VERBOSE(DM_DEBUG_NAME, "%d", connectionData->networkProfiles[i].useDHCP);
        PRINT_VARIABLE(connectionData->networkProfiles[i].staticIP.ip, offset);
        PRINT_VERBOSE(DM_DEBUG_NAME, "%s", ipaddr_ntoa(((const ip_addr_t *)&connectionData->networkProfiles[i].staticIP.ip)));        
        PRINT_VARIABLE(connectionData->networkProfiles[i].staticIP.subnet, offset);
        PRINT_VERBOSE(DM_DEBUG_NAME, "%s", ipaddr_ntoa(((const ip_addr_t *)&connectionData->networkProfiles[i].staticIP.subnet)));        
        PRINT_VARIABLE(connectionData->networkProfiles[i].staticIP.gateway, offset);
        PRINT_VERBOSE(DM_DEBUG_NAME, "%s", ipaddr_ntoa(((const ip_addr_t *)&connectionData->networkProfiles[i].staticIP.gateway)));        
        PRINT_VARIABLE(connectionData->networkProfiles[i].staticIP.dns, offset);
        PRINT_VERBOSE(DM_DEBUG_NAME, "%s", ipaddr_ntoa(((const ip_addr_t *)&connectionData->networkProfiles[i].staticIP.dns)));        
    }
}

void PrintBytesOfObject(const char* name, char* object, size_t size)
{
    PRINT_INFO(DM_DEBUG_NAME, "Byte allocation of: %s", name);
    for(size_t i = 0; i < size; i++)
    {
        if(i != 0)
        {
            printf(", ");
        }

        printf("%#04x (%u)", object[i], object[i]);
    }    
    printf("\n");
}
