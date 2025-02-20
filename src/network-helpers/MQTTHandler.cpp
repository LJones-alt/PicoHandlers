#include "MQTTHandler.h"

void MQTTHandler::Init(const char* connectionString){
    PRINT_INFO(DM_MQTTHANDLER_NAME, "Initialising...");
    _mqttData =(MQTTData::MQTTClient*)calloc(1,sizeof(MQTTData::MQTTClient));
    connectionHelper.Init();
    _creds=(MQTTConnectionHelper::Creds*)calloc(1, sizeof(MQTTConnectionHelper::Creds));
    _creds = connectionHelper.GetConnectionSettings(connectionString);
    PRINT_INFO(DM_MQTTHANDLER_NAME, "Got Connection settings! ");
    _mqttState.client_connected = false;
    _mqttState.client_created =  false;
    _mqttState.gotHostame - false;
    ci = (mqtt_connect_client_info_t*)calloc(1, sizeof(mqtt_connect_client_info_t));
    ci = connectionHelper.CreateClientInfo();
    printf("trying to find the ci info %s", ci->client_id);
    _client = mqtt_client_new();    
    PRINT_VERBOSE(DM_MQTTHANDLER_NAME, "Client pointer: %p", _client);
    PRINT_SUCCESS(DM_MQTTHANDLER_NAME, "Initialised successful, got Device ID: %s", _creds->DeviceName);    
};



void MQTTHandler::UpdateServer(DNSLookup* dns, u16_t port){
    _mqttState.gotHostame = GetIp(dns);
    _creds->Port = port;
};

err_t MQTTHandler::PublishMessage(MQTTMessage::Message* message){
    bool ipok = CheckIP();
    if(!ipok){
        PRINT_ERR(DM_MQTTHANDLER_NAME, "No IP, publish failed ");
        return ERR_CONN;
    }
    PRINT_INFO(DM_MQTTHANDLER_NAME, "publish payload size : %d ", strlen(message->payload));
    message->status = Publish(message);
    if(message->status!=0){
        PRINT_ERR(DM_MQTTHANDLER_NAME, "MQTT publish failed, reason: %d", message->status);
        return  message->status;
    }
    PRINT_SUCCESS(DM_MQTTHANDLER_NAME, "MQTT publish OK ");
    return message->status;
};

bool MQTTHandler::GetIp(DNSLookup* dns){
    
    // lookup and get IP from hostname
    err_t status = dns->UpdateBlocking(); 
    _mqttData->ip_addr = dns->GetIP();  
    if(_mqttData->ip_addr == NULL){
        PRINT_ERR(DM_MQTTHANDLER_NAME, "Couldn't find the IP address for host \n");
        return false;
    }
    PRINT_SUCCESS(DM_MQTTHANDLER_NAME, "Host IP found");
   // dns->PrintIP();
    return _mqttData->ip_addr != NULL;
}

char* MQTTHandler::GetDeviceID(){
    return _creds->DeviceName;
};

/// PRIVATE ////

bool MQTTHandler::UpdateIP(const ip_addr_t* ip){
    _mqttData->_temp_ip_addr = ip;
    return true; 
}

void MQTTHandler::Disconnect(mqtt_client_t *_client){
    PRINT_INFO(DM_MQTTHANDLER_NAME, "mqtt disconnecting...");
    cyw43_arch_lwip_begin();
    mqtt_disconnect(_client);
    cyw43_arch_lwip_end();
    PRINT_SUCCESS(DM_MQTTHANDLER_NAME, "client disconnected");
}

err_t MQTTHandler::ClientConnect(mqtt_client_t* _client){
    PRINT_INFO(DM_MQTTHANDLER_NAME, "MQTT Connecting...");
    if(!CheckIP()){
        _mqttState.error = ERR_CONN;
        return _mqttState.error;
    }
    PRINT_INFO(DM_MQTTHANDLER_NAME, "Server IP ok, connecting...");
    if(_client==NULL){
        PRINT_ERR(DM_MQTTHANDLER_NAME, "Failed to create client");
        return ERR_CONN;
    }
    cyw43_arch_lwip_begin();
    _mqttState.error = mqtt_client_connect(_client, _mqttData->ip_addr, (_creds->Port), mqtt_connection_cb, LWIP_CONST_CAST(void*, ci), ci );
    cyw43_arch_lwip_end();

    return ERR_OK;
};

bool MQTTHandler::CheckIP(){
    if(_mqttData->_temp_ip_addr!=NULL){
        _mqttData->ip_addr = _mqttData->_temp_ip_addr;
        _mqttData->_temp_ip_addr=NULL;
    }
    return(_mqttData->ip_addr !=NULL);
};

err_t MQTTHandler::Publish(MQTTMessage::Message* _message){
    clientBusy = true;
    clientConnecting = true;
    int blockTimeout = 0;
    err_t connectionState = ClientConnect(_client);
    if(_client==NULL){
        PRINT_ERR(DM_MQTTHANDLER_NAME, "Failed to create client");
        return ERR_CONN;
    }
    while(clientConnecting && blockTimeout < MQTT_BLOCKING_THRESHOLD_MS){
        blockTimeout++;
        sleep_ms(1);
    }
    if (blockTimeout >= MQTT_BLOCKING_THRESHOLD_MS){
        PRINT_ERR(DM_MQTTHANDLER_NAME,"could not connect in time...");
        Disconnect(_client);
        _mqttState.error = ERR_TIMEOUT;
        clientBusy=false;
        return _mqttState.error;
    }
    PRINT_VERBOSE(DM_MQTTHANDLER_NAME, "done, connected, now publishing...");
    messageBeingPublished = true;
    blockTimeout = 0;
    cyw43_arch_lwip_begin();
    _mqttState.error = mqtt_publish(_client, _creds->MQTTTopic, _message->payload, strlen(_message->payload) ,QOS, RETAIN, pub_request_cb, NULL);
    cyw43_arch_lwip_end();
    while(messageBeingPublished && blockTimeout < MQTT_BLOCKING_THRESHOLD_MS)
    {
        blockTimeout++;
        sleep_ms(1);
    }
    if (blockTimeout >= MQTT_BLOCKING_THRESHOLD_MS){
        PRINT_ERR(DM_MQTTHANDLER_NAME,"could not publish, timeout...");
        Disconnect(_client);
        _mqttState.error = ERR_TIMEOUT;
        clientBusy=false;
        return _mqttState.error;
    }
    if(_mqttState.error != ERR_OK)
    {
        PRINT_ERR(DM_MQTTHANDLER_NAME,"MQTT Publish failed. Error: %d", _mqttState.error);
        clientBusy=false;
        return _mqttState.error;
    }
    PRINT_SUCCESS(DM_MQTTHANDLER_NAME,"MQTT Publish successful");
    numMessageSent++;
    Disconnect(_client);
    clientBusy=false;
    return ERR_OK;
};

void MQTTHandler::mqtt_connection_cb(mqtt_client_t *client, void *arg, mqtt_connection_status_t status){
    err_t err;
    PRINT_INFO(DM_MQTTHANDLER_NAME,"MQTT Connect Callback - status : %d", status);
    if(status == MQTT_CONNECT_ACCEPTED){
        PRINT_SUCCESS(DM_MQTTHANDLER_NAME,"MQTT Connect Callback - Connected to MQTT");
        clientConnecting = false;
    }
};

void MQTTHandler::pub_request_cb(void *arg, err_t result){
   
    PRINT_INFO(DM_MQTTHANDLER_NAME,"Publish callback got result : %d", result);
    messageBeingPublished = false;
};

