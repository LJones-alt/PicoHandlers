#include "WifiHandler.h"

void WifiHandler::Init(ConnectionSettings::NetworkProfile wifiSettings[SENSIFY_NUMBER_OF_WIFI_PROFILES], const char* deviceId){
    _wifiStatus = DISCONNECTED;
    _wifiSettings = wifiSettings;
    cyw43_wifi_set_up(&cyw43_state, CYW43_ITF_STA, false, CYW43_COUNTRY_WORLDWIDE);        
    cyw43_arch_enable_sta_mode();
    cyw43_wifi_pm(&cyw43_state, CYW43_NO_POWERSAVE_MODE);    

    cyw43_arch_lwip_begin();
    struct netif *n = &cyw43_state.netif[CYW43_ITF_STA];
    netif_set_hostname(n, deviceId);
    netif_set_up(n);
    cyw43_arch_lwip_end();
    
    PRINT_SUCCESS(DM_WIFIHANDLER_NAME, "WiFi initialised - Device will have hostname: '%s'", deviceId);
};

bool WifiHandler::Connect(){
    for(int i=0; i<SENSIFY_NUMBER_OF_WIFI_PROFILES; i++)
    {
        if(strlen(_wifiSettings[i].wifi.ssid) == 0 || strlen(_wifiSettings[i].wifi.pass) == 0)
        {
            //SSID or password blank. Skip.
            continue;
        }

        sleep_ms(10);
        if(Connect(i))
        {
            //If mac address is not set, get it (mac only available on link up
            if(_macAddress[0] == 0)
            {
                uint8_t mac[6] = {0};
                cyw43_wifi_get_mac(&cyw43_state, CYW43_ITF_STA,mac);

                snprintf(_macAddress, sizeof(_macAddress),
                "%02x:%02x:%02x:%02x:%02x:%02x",
                mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
            }

            //Connected
            return true;
        }
        //Not connected
    }

    //All connections attempted, returning failed
    PRINT_ERR(DM_WIFIHANDLER_NAME,"No Wifi Found");
    return false;
}

bool WifiHandler::Connect(int index){
    _wifiStatus = CONNECTING; 
    lastAttemptedProfile = index;
    ConnectionSettings::NetworkProfile *settings = &_wifiSettings[index];
    int blockCounter = 0;

    cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, false);        

    PRINT_INFO(DM_WIFIHANDLER_NAME,"Trying to connect to SSID: %s",settings->wifi.ssid);    

    if(settings->useDHCP)
    {        
        SetDHCP();
    }
    else
    {
        SetStaticIP(&settings->staticIP.ip, &settings->staticIP.subnet, &settings->staticIP.gateway, &settings->staticIP.dns);
    }    

    int wifiReturnCode = cyw43_arch_wifi_connect_timeout_ms(settings->wifi.ssid, settings->wifi.pass, CYW43_AUTH_WPA2_MIXED_PSK, 15000);    

    if (wifiReturnCode < CYW43_LINK_DOWN) {
        PRINT_ERR(DM_WIFIHANDLER_NAME,"Failed to connect to network (%d)", wifiReturnCode);
        cyw43_wifi_leave(&cyw43_state, CYW43_ITF_STA);
        return false;
    }
    else { 
        PRINT_INFO(DM_WIFIHANDLER_NAME,"Connected - Waiting for link-up and IP");
        while(netif_is_link_up(cyw43_state.netif) == 0)
        {          
            if(blockCounter++ > blockCounterThreshold)
            {
                PRINT_ERR(DM_WIFIHANDLER_NAME,"Failed to join network - No Link-up");
                _wifiStatus = DISCONNECTED;
                cyw43_wifi_leave(&cyw43_state, CYW43_ITF_STA);
                return false;
            }
            sleep_ms(10);
        }
        blockCounter = 0;

        while(netif_ip4_addr(cyw43_state.netif)->addr == IPADDR_ANY)
        {
            if(blockCounter++ > blockCounterThreshold)
            {
                PRINT_ERR(DM_WIFIHANDLER_NAME,"Failed to join network - No IP");
                _wifiStatus = DISCONNECTED;   
                cyw43_wifi_leave(&cyw43_state, CYW43_ITF_STA);
                return false;
            }
            sleep_ms(10);
        }
        sleep_ms(50);
        PRINT_SUCCESS(DM_WIFIHANDLER_NAME,"Connected to WiFI - IP:%s (%s)", ipaddr_ntoa(netif_ip4_addr(cyw43_state.netif)),netif_get_hostname(cyw43_state.netif));
        cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, true);

        _wifiStatus = CONNECTED;
        return true;
    }    
};

bool WifiHandler::ReConnect(){
    cyw43_wifi_leave(&cyw43_state, CYW43_ITF_STA);
    sleep_ms(250);
    return Connect();
}

WifiHandler::WIFI_STATUS WifiHandler::GetStatus()
{    
    return _wifiStatus;
};

int WifiHandler::GetConnectedProfile()
{    
    if(GetStatus() == CONNECTED)
    {
        return lastAttemptedProfile;
    }
    else
    {
        return -1;
    }
};

uint8_t WifiHandler::GetSignalStrength()
{
    int32_t rssi;
    int ret = cyw43_ioctl(&cyw43_state, 254, sizeof rssi, (uint8_t *)&rssi, CYW43_ITF_STA);
    return rssi;
}

void WifiHandler::SetDHCP()
{
    if(_dhcpEnabled == false)
    {
        _dhcpEnabled = true;
        dhcp_start(cyw43_state.netif);    
    }    
}

void WifiHandler::SetStaticIP(ip4_addr* ip, ip4_addr* netmask, ip4_addr* gw, ip4_addr* dns)
{
    if(_dhcpEnabled == true)
    {
        dhcp_stop(cyw43_state.netif);
        _dhcpEnabled = false;
    }
    netif_set_addr(cyw43_state.netif, ip, netmask, gw);
    if(dns != NULL)
    {
        DNSLookup::SetPrimaryDNSServer(dns);
    }    
}

const char* WifiHandler::GetMacAddress()
{
    return _macAddress;
}