// src/BLE/Ch58xBleAdvertiser.cpp
#include "BLE/Ch58xBleAdvertiser.h"
#include "BLE/Ch58xBlePlatform.h"

extern "C" {
#include "CH58xBLE_LIB.H"
#include "CH58x_common.h"
}

namespace {
bool get_role_state(gapRole_States_t &state) {
  return GAPRole_GetParameter(GAPROLE_STATE, &state) == SUCCESS;
}
} // namespace

BleResult Ch58xBleAdvertiser::init() {
  if (initialized_)
    return BleResult::Success;
  if (!Ch58xBlePlatform::init_ble_stack())
    return BleResult::HardwareError;

  uint8_t mac[6] = {0};
  GetMACAddress(mac);
  GAPRole_SetParameter(GAPROLE_BD_ADDR, B_ADDR_LEN, mac);

  initialized_ = true;
  return BleResult::Success;
}

BleResult Ch58xBleAdvertiser::set_payload(AdvPayloadType type,
                                          ByteSpan payload) {
  if (!ready()) {
    return BleResult::NotInitialized;
  }

  if (payload.size() > kMaxLegacyAdvLen) {
    return BleResult::PayloadTooLarge;
  }

  if (payload.size() != 0 && payload.data() == nullptr) {
    return BleResult::InvalidParam;
  }

  const uint16_t param = (type == AdvPayloadType::AdvertisingData)
                             ? GAPROLE_ADVERT_DATA
                             : GAPROLE_SCAN_RSP_DATA;

  const uint8_t len = static_cast<uint8_t>(payload.size());

  const bStatus_t roleStatus =
      GAPRole_SetParameter(param, len, const_cast<uint8_t *>(payload.data()));

  if (roleStatus != SUCCESS) {
    return BleResult::HardwareError;
  }

  return BleResult::Success;
}

BleResult Ch58xBleAdvertiser::set_interval(std::chrono::milliseconds interval) {
  if (!ready())
    return BleResult::NotInitialized;
  if (!is_valid_interval(interval))
    return BleResult::InvalidInterval;

  const uint16_t ticks = ms_to_ble_ticks(interval);
  GAP_SetParamValue(TGAP_DISC_ADV_INT_MIN, ticks);
  GAP_SetParamValue(TGAP_DISC_ADV_INT_MAX, ticks);
  return BleResult::Success;
}

BleResult Ch58xBleAdvertiser::start() {
  if (!ready())
    return BleResult::NotInitialized;
  uint8_t enabled = TRUE;
  if (GAPRole_SetParameter(GAPROLE_ADVERT_ENABLED, sizeof(enabled), &enabled) !=
      SUCCESS) {
    return BleResult::HardwareError;
  }
  return BleResult::Success;
}

BleResult Ch58xBleAdvertiser::stop() {
  if (!ready())
    return BleResult::NotInitialized;
  uint8_t enabled = FALSE;
  if (GAPRole_SetParameter(GAPROLE_ADVERT_ENABLED, sizeof(enabled), &enabled) !=
      SUCCESS) {
    return BleResult::HardwareError;
  }
  return BleResult::Success;
}

bool Ch58xBleAdvertiser::is_advertising() const noexcept {
  if (!ready())
    return false;
  gapRole_States_t state = GAPROLE_INIT;
  if (!get_role_state(state))
    return false;
  return state == GAPROLE_ADVERTISING;
}