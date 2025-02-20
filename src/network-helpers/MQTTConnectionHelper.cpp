#include "MQTTConnectionHelper.h"

void MQTTConnectionHelper::Init(){
    PRINT_INFO(DM_MQTTCONNECTIONHELPER_NAME, "Starting MQTT Connection Handler");
    // hold this memory location for the mqtt connection credentials 
    _creds = (MQTTConnectionHelper::Creds*)calloc(1, sizeof(MQTTConnectionHelper::Creds));
    _connectionString = new char[512];
}

MQTTConnectionHelper::Creds* MQTTConnectionHelper::GetConnectionSettings(const char* connectionString){
    PRINT_INFO(DM_MQTTCONNECTIONHELPER_NAME, "Getting MQTT connection info");
    //MQTTConnectionHelper::Creds* _creds = (MQTTConnectionHelper::Creds*)calloc(1, sizeof(MQTTConnectionHelper::Creds));
    _connectionString = new char[512];
        // string pointer copy to connection string 
    strcpy(_connectionString, connectionString);
    GetConnectionSettings();
    return _creds;
}

// hopefully this will create a client info object
mqtt_connect_client_info_t* MQTTConnectionHelper::CreateClientInfo(){
    mqtt_connect_client_info_t* ci = (mqtt_connect_client_info_t*)calloc(1,sizeof(mqtt_connect_client_info_t));
    ci->client_id= _creds->DeviceName;
    ci->will_topic=""; // create, but do not populate
    ci->client_user= _creds->MQTTUser;
    ci->client_pass= _creds->SASToken;
    ci->tls_config= altcp_tls_create_config_client(NULL, 0);
    PRINT_INFO(DM_MQTTCONNECTIONHELPER_NAME,"Found CI : Device name is %s", ci->client_id);
    return ci;
};

                    ///////////////// -- private --- //////////         
char* MQTTConnectionHelper::SplitOnSymbol(char* str){
      for(size_t i = 0; i < strlen(str); i++)
    {
        if(str[i] == '=') return str + i + 1;
    }
    return NULL;
}

void MQTTConnectionHelper::GetConnectionSettings(){
    /*char* str = new char[strlen(connectionString)+1];
    strcpy(str, connectionString);*/
	char delim[] = ";";
    char* _temp[3]={};
	char *ptr = strtok(_connectionString, delim);
    int i=0;
	while(ptr != NULL)
	{
        _temp[i]=ptr;
		ptr = strtok(NULL, delim);
        i++;
	}
    _creds->Hostname=SplitOnSymbol(_temp[0]);
    _creds->DeviceName=SplitOnSymbol(_temp[1]);
    _creds->expiry = (long)Time::GetUnixTimestamp + 100000000L;
    _creds->SASToken = DecodeBase64(SplitOnSymbol(_temp[2]));
    _creds->MQTTUser = (char*) calloc(100, sizeof(char));
    _creds->MQTTTopic = (char*) calloc(100, sizeof(char));
    int ret = snprintf(_creds->MQTTUser, 100, "%s/%s/?api-version=2021-04-12",_creds->Hostname,_creds->DeviceName );
    ret = snprintf(_creds->MQTTTopic, 100, "devices/%s/messages/events/$.ct=application%%2Fjson%%3Bcharset%%3Dutf-8", _creds->DeviceName );
    PRINT_SUCCESS(DM_MQTTCONNECTIONHELPER_NAME, "Generated MQTT client info for device ID %s", _creds->DeviceName);
}


char* MQTTConnectionHelper::DecodeBase64(char input_str[])
{
    PRINT_INFO(DM_MQTTCONNECTIONHELPER_NAME, "Decoding string from base64 ");
    char* decoded = (char*) calloc(MQTT_BASE64_BUFFER_SIZE, sizeof(char));
    int ret = mbedtls_base64_decode((unsigned char*)decoded,MQTT_BASE64_BUFFER_SIZE, 0, (unsigned char*)input_str, strlen(input_str));
    return decoded;
}

