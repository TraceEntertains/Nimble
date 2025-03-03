#include <stdio.h>
#include <mocha/mocha.h>

#include <string>

#include "cJSON/cJSON.h"
#include "log.h"
#include "checkInkayStatus.hpp"

static int CheckInkayJson(const char* jsonBuffer) {
	// Parse the JSON string
    cJSON* root = cJSON_Parse(jsonBuffer);
	
    if (root == NULL) {
		return -1;
    } 
	
	// Get the "storageitems" object
	cJSON* storageitems = cJSON_GetObjectItem(root, "storageitems");
	
	if (storageitems == NULL) {
		// Free the root node
		cJSON_Delete(root);
		
		return -1;
	}
	
	// Get the value of "connect_to_network" key
	cJSON* connect_to_network = cJSON_GetObjectItem(storageitems, "connect_to_network");
	
	if (!cJSON_IsNumber(connect_to_network)) {
		// Free the root node
		cJSON_Delete(root);
		
		return -1;
	} 
	
	// Free the root node
	cJSON_Delete(root);
	
	return static_cast<bool>(connect_to_network->valueint);
}

bool skipPatches() {
	std::string inkayConfigPath = "/plugins/config/inkay.json";

    char environmentPathBuffer[0x100];
    MochaUtilsStatus status;
    if ((status = Mocha_GetEnvironmentPath(environmentPathBuffer, sizeof(environmentPathBuffer))) != MOCHA_RESULT_SUCCESS) {
		log("Failed to get the environment path. Error: %s", Mocha_GetStatusStr(status));
    }

	FILE* configFile = fopen((environmentPathBuffer + inkayConfigPath).c_str(), "r");
	if (configFile == NULL) {
		log("Inkay config file does not exist");
		return true;
	}
	
	fseek(configFile, 0, SEEK_END); // seek to end of file
	int size = ftell(configFile); // get file pointer location (for size)
	fseek(configFile, 0, SEEK_SET); // seek back to beginning of file
	
	if (size > MAX_BUFFER_SIZE) {
		log("Inkay config file is too large");
		return true;
	}
	
	char buffer[MAX_BUFFER_SIZE];
	size_t bytesRead = fread(buffer, 1, MAX_BUFFER_SIZE, configFile);
	if (bytesRead == 0) {
		log("Inkay config file is empty");
		return true;
	}
	
	const char* const_buffer = buffer;
	
	int result = CheckInkayJson(const_buffer);
	if (result == -1) {
		log("Inkay config file is invalid");
		return true;
	}
	
	return !result; // return !result because if skipPatches is true, it skips patches, and we *don't* want to skip patches if connect_to_network is 1, so invert it
}