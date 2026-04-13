#include <Arduino.h>
#include <device_status.h>
#include <carrier_utilities.h>

void state_init(CarrierUtilities& carrUtil, String devId);
void saveLastState(DeviceState newState);

DeviceState handleStartup();
DeviceState handleRetrieveToken(String* outToken);

DeviceState handleDisconnected();
DeviceState handleConnected(SensorData sensorData, bool& updateScreen, unsigned long now);
DeviceState handleHeartbeatError(unsigned long now);
DeviceState handleDataError(SensorData sensorData);
DeviceState handleTokenError();
DeviceState handleWifiError();

