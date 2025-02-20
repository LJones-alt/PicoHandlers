#include "debug/PrettySerial.h"
#include "network-helpers/ConnectionSettings.h"
#include "lwip/ip_addr.h"

#define DM_DEBUG_NAME "DEBUG"

#define PRINT_VARIABLE(var, offset)\
    PRINT_INFO(DM_DEBUG_NAME, "%s:\tStart: %lu \tSize: %lu\t End: %lu", #var, ((char *) &var - offset), sizeof(var), ((char *) &var - offset) + sizeof(var));        

void TestApplication(ConnectionSettings::ConnectionData *connectionData);
void PrintConnectionStringMemoryAllocation(ConnectionSettings::ConnectionData *connectionData);
void PrintBytesOfObject(const char* name, char* object, size_t size);