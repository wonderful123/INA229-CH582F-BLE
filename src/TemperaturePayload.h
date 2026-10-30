#pragma once

#include "PayloadBuilder.h"

namespace TemperaturePayload {

// Builds the complete BLE advertising packet for the temperature sensor.
//
// The temperature is read from Ch58xPlatform internally.
// Returns an invalid Advertisement if the packet cannot be built.
PayloadBuilder::Advertisement build();

} // namespace TemperaturePayload