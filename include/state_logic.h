#include <Arduino.h>
#include <device_status.h>
#include <carrier_utilities.h>
#include <carrier_wifi.h>

void state_init(CarrierUtilities& carrUtil, CarrierWiFi& carrWifi, String devId);
void saveLastState(DeviceState newState);

DeviceState handleStartup();
DeviceState handleToken(String* outToken);

DeviceState handleDisconnected();
DeviceState handleConnected(SensorData sensorData, bool& updateScreen);
DeviceState handleHeartbeatError();
DeviceState handleDataError(SensorData sensorData);
DeviceState handleTokenError();
DeviceState handleWifiError();

