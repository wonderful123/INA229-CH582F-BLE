#include "TemperaturePayload.h"

#include "System/Ch58xPlatform.h"

#include <cstdint>

namespace TemperaturePayload {

namespace {

constexpr uint8_t kAdTypeFlags = 0x01;
constexpr uint8_t kAdTypeCompleteLocalName = 0x09;
constexpr uint8_t kAdTypeManufacturerData = 0xFF;

constexpr uint8_t kFlags = 0x06;
constexpr uint16_t kCompanyId = 0xFFFF;
constexpr uint8_t kProtocolVersion = 0x01;

constexpr char kDeviceName[] = "CH58x";

} // namespace

PayloadBuilder::Advertisement build() {
  const int16_t temperature_centi = Ch58xPlatform::temperature();
  const uint16_t temperature_raw = static_cast<uint16_t>(temperature_centi);

  const uint8_t manufacturer_data[] = {
      static_cast<uint8_t>(kCompanyId & 0xFF),
      static_cast<uint8_t>(kCompanyId >> 8),
      static_cast<uint8_t>(temperature_raw & 0xFF),
      static_cast<uint8_t>(temperature_raw >> 8),
      kProtocolVersion,
  };

  PayloadBuilder::Builder builder;

  if (!builder.append(kAdTypeFlags, &kFlags, sizeof(kFlags))) {
    return {};
  }

  if (!builder.append(kAdTypeCompleteLocalName,
                      reinterpret_cast<const uint8_t *>(kDeviceName),
                      sizeof(kDeviceName) - 1)) {
    return {};
  }

  if (!builder.append(kAdTypeManufacturerData, manufacturer_data,
                      sizeof(manufacturer_data))) {
    return {};
  }

  return builder.finish();
}

} // namespace TemperaturePayload