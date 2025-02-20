#ifndef DMNTP_H
#define DMNTP_H

#include "pico/stdlib.h"
#include "pico/util/datetime.h"
#include <string.h>
#include <time.h>
#include "pico/cyw43_arch.h"
#include "lwip/dns.h"
#include "lwip/pbuf.h"
#include "lwip/udp.h"
#include "DNSLookup.h"
#include "Sensify-config.h"
#include "PrettySerial.h"
#include "network-helpers/ConnectionSettings.h"

#define DM_NTP_NAME "NTP"

class NTP
{
	public:                
        typedef struct NTP_T_ {
            ip_addr_t ntp_server_address;
            bool dns_request_sent;
            bool success;
            struct udp_pcb *ntp_pcb;
            absolute_time_t ntp_test_time;
            alarm_id_t ntp_resend_alarm;
            uint32_t seconds;
            tm *lastResultUTC;
        } NTP_T;

        NTP(ConnectionSettings::NTPSettings *ntpServer);
        ~NTP();
        void Query();
        NTP::NTP_T* GetResult();
        tm* QueryBlocking(); 

    private:        

        NTP_T* _state;
        DNSLookup* _dns;        

        NTP_T* Init();
        static void ResultCallback(NTP_T* state, int status, time_t *result);
        static int64_t FailedHandler(alarm_id_t id, void *user_data);
        static void Request(NTP_T* state);
        static void DataReceived(void *arg, struct udp_pcb *pcb, struct pbuf *p, const ip_addr_t *addr, u16_t port);
        static void DnsFoundCallback(const char *hostname, const ip_addr_t *ipaddr, void *arg);
};

#endif