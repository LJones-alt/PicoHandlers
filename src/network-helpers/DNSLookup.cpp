#include "DNSLookup.h"

DNSLookup::DNSLookup(const char* hostname, ip_addr_t* fallback, uint16_t blockingTimeout)
{
    dnsResult = (DnsResult*)calloc(1, sizeof(DnsResult));
    _hostnameEmpty = (strlen(hostname)) == 0;

    if(fallback == NULL || fallback->addr == IPADDR_NONE)
    {
        dnsResult->error = ERR_VAL;
    }
    else
    {
        _fallbackIP = fallback;
        dnsResult->resolvedIP = *fallback;
        dnsResult->error = ERR_CONN;
    }    
    _hostname = hostname;
    _blockTimeout = blockingTimeout / 10;

    //Check configured DNS servers
    for(u8_t i = 0; i< DNS_MAX_SERVERS; i++)
    {
        if(dns_getserver(i) == IP_ADDR_ANY)
        {
            //Slot empty, adding backup server
            dns_setserver(i, &_fallbackDNSServer);
            break;
        }
    }
}

DNSLookup::~DNSLookup()
{
    free(dnsResult);

}

void DNSLookup::DnsCallback(const char *hostname, const ip_addr_t *ipaddr, void *arg)
{
    DnsResult *ipStruct = (DnsResult*)arg;
    if (ipaddr) {  
        ipStruct->resolvedIP = *ipaddr;
        ipStruct->error = ERR_OK;
    } else {
        ipStruct->resolvedIP;
        ipStruct->error = ERR_TIMEOUT;
    }
}

err_t DNSLookup::UpdateBlocking()
{
    if(_hostnameEmpty)
    {
        //Hostname is invalid, skip lookup.
        dnsResult->error == ERR_ARG;
    }
    else
    {
        //Perform DNS lookup
        cyw43_arch_lwip_begin();
        dnsResult->error = dns_gethostbyname(_hostname, &dnsResult->resolvedIP, DnsCallback, dnsResult);    
        cyw43_arch_lwip_end();
        if(dnsResult->error != ERR_OK)    
        {
            //Not OK.. block for a while waiting for callback
            for(int i = 0; i<_blockTimeout; i++)
            {
                sleep_ms(10);
                //printf("WAAAAITING (%d) - %d...\n",i, dnsResult->error);
                if(dnsResult->error == ERR_OK)
                {
                    break;
                }
            }        
        }
        else
        {
            //Everything is fine? Ok.
        }
    }    
    return dnsResult->error;
}

/*
UNTESTED and Unused???
err_t DNSLookup::UpdateBackground()
{    
    return dns_gethostbyname(_hostname, &dnsResult->resolvedIP, DnsCallback, dnsResult);
}
*/


err_t DNSLookup::GetIP(ip_addr_t *ipaddr)
{
    ip_addr_copy(*ipaddr, *GetIP());
    return dnsResult->error;
}

ip_addr_t* DNSLookup::GetIP()
{
    if(dnsResult->error == ERR_OK)
    {
        return &dnsResult->resolvedIP;
    }
    else if(_fallbackIP != NULL && _fallbackIP->addr != IPADDR_NONE)
    {
        //Dns lookup failed, falling over successfully to fallback
        dnsResult->error = ERR_OK;
        return _fallbackIP;
    }
    else
    {
        return NULL;
    }
}

const ip_addr_t* DNSLookup::GetPrimaryDNSServer()
{
    return dns_getserver(0);    
}

void DNSLookup::SetPrimaryDNSServer(const ip_addr_t* dnsServer)
{
    dns_setserver(0, dnsServer);
}

void DNSLookup::PrintIP()
{
    PRINT_INFO(DM_DNSLOOKUP_NAME, "DNS Result for %s is ip:%s (err:%d )\n",_hostname, ipaddr_ntoa(((const ip_addr_t *)&dnsResult->resolvedIP)), dnsResult->error);
}

