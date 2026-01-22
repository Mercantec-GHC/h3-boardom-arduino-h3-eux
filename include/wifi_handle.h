#include <Arduino.h>
#include <Arduino_MKRIoTCarrier.h>
#include <carrier_utilities.h>

bool wifi_Init(CarrierUtilities carrUtil, uint16_t timeoutMs);
String wifi_GetDeviceID();
bool wifi_HttpPost(const char* endpoint, String jsonBody, String& response, const char server_ip, uint16_t server_port);
