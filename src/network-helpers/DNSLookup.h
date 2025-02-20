#ifndef DM_DNSLOOKUP_H
#define DM_DNSLOOKUP_H
#include "lwip/dns.h"
#include "debug/PrettySerial.h"
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"

#define DM_DNSLOOKUP_NAME "DNS"

class DNSLookup
{
	public:        
        DNSLookup(const char* hostname, ip_addr_t* fallback, uint16_t blockingTimeout);
        DNSLookup(const char* hostname, uint16_t blockingTimeout) : DNSLookup(hostname, NULL, blockingTimeout){};
        DNSLookup(const char* hostname) : DNSLookup(hostname, NULL, 5000){};
        ~DNSLookup();
        
        err_t UpdateBlocking();
        //err_t UpdateBackground();
        err_t GetIP(ip_addr_t* ipAddress);
        ip_addr_t* GetIP();
        static const ip_addr_t* GetPrimaryDNSServer();
        static void SetPrimaryDNSServer(const ip_addr_t* dnsServer);
        void PrintIP();

    private:

        struct DnsResult {            
            ip_addr_t resolvedIP;    
            err_t error;
        };

        const char* _hostname;
        ip_addr_t* _fallbackIP = NULL;
        DnsResult *dnsResult;
        bool _hostnameEmpty = false;

        uint16_t _blockTimeout;

        const ip_addr_t _fallbackDNSServer = IPADDR4_INIT(0x08080808);  //8.8.8.8

        static void DnsCallback(const char *hostname, const ip_addr_t *ipaddr, void *arg);
};
 #endif