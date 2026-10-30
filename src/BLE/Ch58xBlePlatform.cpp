#include "Ch58xBlePlatform.h"
#include "System/Ch58xPlatform.h"

extern "C" {
#include "CH58xBLE_LIB.H"
#include "CH58x_common.h"
}

namespace {

alignas(4) uint32_t g_ble_memory[10 * 1024 / 4];
gapRolesBroadcasterCBs_t g_broadcaster_callbacks{nullptr, nullptr};

} // namespace

namespace Ch58xBlePlatform {

bool init_ble_stack() {
  bleConfig_t config{};
  config.MEMAddr = reinterpret_cast<uint32_t>(g_ble_memory);
  config.MEMLen = sizeof(g_ble_memory);
  config.BufNumber = 5;
  config.BufMaxLen = 27;
  config.TxNumEvent = 1;
  config.RxNumEvent = 5;
  config.TxPower = LL_TX_POWEER_0_DBM;
  config.SelRTCClock = 1;
  config.srandCB = Ch58xPlatform::random_seed;
  config.sleepCB = Ch58xPlatform::sleep_ticks;
  config.tsCB = Ch58xPlatform::temperature;

  if (BLE_LibInit(&config) != SUCCESS)
    return false;

  TMOS_TimerInit(nullptr);

  if (GAPRole_BroadcasterInit() != SUCCESS)
    return false;

  if (GAPRole_BroadcasterStartDevice(&g_broadcaster_callbacks) != SUCCESS)
    return false;

  return true;
}

} // namespace Ch58xBlePlatform
