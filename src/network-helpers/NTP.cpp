/**
 * Copyright (c) 2022 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "NTP.h"

#define NTP_MSG_LEN 48
#define NTP_PORT 123
#define NTP_DELTA 2208988800 // seconds between 1 Jan 1900 and 1 Jan 1970
#define NTP_TEST_TIME (30 * 1000)
#define NTP_RESEND_TIME (10 * 1000)

NTP::NTP(ConnectionSettings::NTPSettings *ntpServer)
{       
    _dns = new DNSLookup(ntpServer->ntpFQDN,&ntpServer->fallbackIP,5000);    
    _state = Init();
}

NTP::~NTP()
{  
    if(_state != NULL)
    {
        udp_remove(_state->ntp_pcb);
        free(_state);
    }
    delete _dns;
}

// Perform initialisation
NTP::NTP_T* NTP::Init() {
    NTP_T *state = (NTP_T*)calloc(1, sizeof(NTP_T));
    if (!state) {
        PRINT_ERR(DM_NTP_NAME, "failed to allocate _state");
        return NULL;        
    }
    state->ntp_pcb = udp_new_ip_type(IPADDR_TYPE_ANY);
    if (!state->ntp_pcb) {
        PRINT_ERR(DM_NTP_NAME, "failed to create pcb");
        free(state);
        return NULL;
    }
    udp_recv(state->ntp_pcb, DataReceived, state);
    return state;
}

// Called with results of operation
void NTP::ResultCallback(NTP_T* state, int status, time_t *result)
{
    if (status == 0 && result) {
        state->lastResultUTC = gmtime(result);
        PRINT_SUCCESS(DM_NTP_NAME, "got ntp response: %02d/%02d/%04d %02d:%02d:%02d\n", state->lastResultUTC ->tm_mday, state->lastResultUTC ->tm_mon + 1, state->lastResultUTC ->tm_year + 1900,
               state->lastResultUTC ->tm_hour, state->lastResultUTC ->tm_min, state->lastResultUTC->tm_sec);
        state->success = true;
    }

    if (state->ntp_resend_alarm > 0) {
        cancel_alarm(state->ntp_resend_alarm);
        state->ntp_resend_alarm = 0;
    }
    state->ntp_test_time = make_timeout_time_ms(NTP_TEST_TIME);
    state->dns_request_sent = false;
}

// Make an NTP request
void NTP::Request(NTP_T* state)
{
    cyw43_arch_lwip_begin();
    struct pbuf *p = pbuf_alloc(PBUF_TRANSPORT, NTP_MSG_LEN, PBUF_RAM);
    uint8_t *req = (uint8_t *) p->payload;
    memset(req, 0, NTP_MSG_LEN);
    req[0] = 0x1b;
    udp_sendto(state->ntp_pcb, p, &state->ntp_server_address, NTP_PORT);
    pbuf_free(p);
    cyw43_arch_lwip_end();
}

int64_t NTP::FailedHandler(alarm_id_t id, void *user_data)
{
    NTP_T* _state = (NTP_T*)user_data;
    PRINT_ERR(DM_NTP_NAME, "ntp request failed");
    ResultCallback(_state, -1, NULL);
    return 0;
}

// NTP data received
void NTP::DataReceived(void *arg, struct udp_pcb *pcb, struct pbuf *p, const ip_addr_t *addr, u16_t port)
{
    NTP_T *_state = (NTP_T*)arg;
    uint8_t mode = pbuf_get_at(p, 0) & 0x7;
    uint8_t stratum = pbuf_get_at(p, 1);

    // Check the result
    if (ip_addr_cmp(addr, &_state->ntp_server_address) && port == NTP_PORT && p->tot_len == NTP_MSG_LEN &&
        mode == 0x4 && stratum != 0) {
        uint8_t seconds_buf[4] = {0};
        pbuf_copy_partial(p, seconds_buf, sizeof(seconds_buf), 40);
        uint32_t seconds_since_1900 = seconds_buf[0] << 24 | seconds_buf[1] << 16 | seconds_buf[2] << 8 | seconds_buf[3];
        uint32_t seconds_since_1970 = seconds_since_1900 - NTP_DELTA;
        time_t epoch = seconds_since_1970;
        _state->seconds = seconds_since_1970;
        ResultCallback(_state, 0, &epoch);
    } else {
        PRINT_ERR(DM_NTP_NAME,"Invalid ntp response");
        ResultCallback(_state, -1, NULL);
    }
    pbuf_free(p);
}

void NTP::Query()
{    
    PRINT_INFO(DM_NTP_NAME, "Getting time from NTP");
    _dns->UpdateBlocking();    
    if(_dns->GetIP(&_state->ntp_server_address) == ERR_OK)
    {
        Request(_state);
        _state->dns_request_sent = true;
        _state->success = false;
    }    
}
NTP::NTP_T* NTP::GetResult()
{
    return _state;
}

tm* NTP::QueryBlocking()
{
    Query();

    int retries = 0;
    for(int i=0; i<NTP_RETRIES; i++)
    {    
        int blockTimeout = 0;
        while(_state->dns_request_sent && blockTimeout < NTP_BLOCKING_THRESHOLD_MS)
        {
            blockTimeout++;
            sleep_ms(1);
        }

        if(_state->success)
        {
            return GetResult()->lastResultUTC;
        }
        PRINT_WARN(DM_NTP_NAME, "NTP Retry %d/%d",i,NTP_RETRIES);
    }
    return NULL;
}