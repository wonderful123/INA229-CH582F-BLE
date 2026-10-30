#include <array>
#include <chrono>
#include <cstdint>

#include "BLE/Ch58xBleAdvertiser.h"
#include "Hardware/CH582F/CH582FGpioOutput.hpp"
#include "Hardware/CH582F/CH582FSpi.hpp"
#include "Hardware/PowerSensors.hpp"
#include "Hardware/Sensor/Ina229SpiAdapter.hpp"
#include "PayloadBuilder.h"
#include "PowerPayload.h"
#include "System/Ch58xPlatform.h"

extern "C" {
#include "CH58x_common.h"
#include "CH58xBLE_LIB.H"
}

namespace {

constexpr uint32_t kSensorUpdateTicks = 1600; // 1 second
constexpr uint32_t kMaxSleepTicks = 80;       // 50 ms

constexpr uint32_t kSolarCsPin = GPIO_Pin_10;
constexpr uint32_t kBatteryCsPin = GPIO_Pin_11;
constexpr uint32_t kLoadCsPin = GPIO_Pin_12;

using CsPin = Hardware::CH582FGpioOutput<Hardware::CH582Port::PortA>;

// Bundles the pin-owning objects so their setup can be re-applied.
struct Board {
  Hardware::CH582FSpi &spi;
  CsPin &solarCs;
  CsPin &batteryCs;
  CsPin &loadCs;

  void reinit() {
    spi.init();
    solarCs.init();
    batteryCs.init();
    loadCs.init();
  }
};

uint32_t tmos_to_rtc_ticks(uint32_t ticks) noexcept {
  return (ticks * 512u) / 25u;
}

void sleep_until_next_event(uint32_t elapsedTicks) {
  const uint32_t remaining = kSensorUpdateTicks - elapsedTicks;

  const uint32_t sleepTicks =
      remaining < kMaxSleepTicks ? remaining : kMaxSleepTicks;

  if (sleepTicks != 0) {
    Ch58xPlatform::sleep_ticks(tmos_to_rtc_ticks(sleepTicks));
  }
}

// Scan response: the TX power level, 0 dBm.
PayloadBuilder::Advertisement make_scan_response() noexcept {
  PayloadBuilder::Builder builder;
  builder.append(PayloadBuilder::AdType::kTxPowerLevel,
                 std::array<uint8_t, 1>{0});
  return builder.finish();
}

// Reads the sensors, publishes the next packet, then retries any chip that
// failed to initialise.
void update_advertisement(Ch58xBleAdvertiser &advertiser, Board &board,
                          Hardware::PowerSensors &sensors,
                          PowerPayload::Encoder &encoder) {
  // Re-apply the SPI and CS pin setup before every read.
  board.reinit();

  // The sensors are read here, while the packet is encoded.
  const PayloadBuilder::Advertisement packet = encoder.next(sensors);

  advertiser.set_payload(AdvPayloadType::AdvertisingData,
                         {packet.data(), packet.size()});

  // Retry chips that failed to initialise. Done after the read so a chip that
  // starts here has a full second to finish its first conversion.
  (void)sensors.init();
}

} // namespace

int main() {
  SetSysClock(CLK_SOURCE_PLL_60MHz);
  Ch58xPlatform::init_system();

  Hardware::CH582FSpi spi;
  CsPin solarCs{kSolarCsPin};
  CsPin batteryCs{kBatteryCsPin};
  CsPin loadCs{kLoadCsPin};

  Board board{spi, solarCs, batteryCs, loadCs};
  board.reinit();

  Hardware::Ina229SpiAdapter solarSpi{spi, solarCs};
  Hardware::Ina229SpiAdapter batterySpi{spi, batteryCs};
  Hardware::Ina229SpiAdapter loadSpi{spi, loadCs};

  Hardware::PowerSensors sensors{
      Hardware::PowerBuses{solarSpi, batterySpi, loadSpi}};

  // Starts the conversions. A failed init does not stop the firmware: failed
  // chips are retried every cycle.
  (void)sensors.init();

  PowerPayload::Encoder encoder;

  Ch58xBleAdvertiser advertiser;
  advertiser.init();
  advertiser.set_interval(std::chrono::milliseconds{100});

  const PayloadBuilder::Advertisement scanResponse = make_scan_response();
  advertiser.set_payload(AdvPayloadType::ScanResponseData,
                         {scanResponse.data(), scanResponse.size()});

  // Advertising starts after the first reading (about 1 s after boot). The
  // chips need about 0.5 s for their first averaged result, so an earlier
  // packet would carry zeros.
  bool advertising = false;

  uint32_t lastUpdate = TMOS_GetSystemClock();

  while (true) {
    TMOS_SystemProcess();

    const uint32_t now = TMOS_GetSystemClock();
    const uint32_t elapsed = static_cast<uint32_t>(now - lastUpdate);

    if (elapsed >= kSensorUpdateTicks) {
      lastUpdate = now;

      update_advertisement(advertiser, board, sensors, encoder);

      if (!advertising) {
        advertiser.start();
        advertising = true;
      }

      continue;
    }

    sleep_until_next_event(elapsed);
  }
}