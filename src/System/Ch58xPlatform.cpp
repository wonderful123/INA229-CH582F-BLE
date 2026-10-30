#include "Ch58xPlatform.h"

extern "C" {
#include "CH58x_common.h"
#include "HAL.h"
}

namespace Ch58xPlatform {

void init_system() {
  HAL_Init();
  HAL_SleepInit();
}

uint32_t random_seed() { return SYS_GetSysTickCnt() ^ RTC_GetCycle32k(); }

uint32_t sleep_ticks(uint32_t ticks) {
  if (ticks == 0)
    return 0;
  return CH58X_LowPower(RTC_GetCycle32k() + ticks);
}

// -----------------------------------------------------------------------------
// Correct CH58x temperature conversion
// -----------------------------------------------------------------------------

static int16_t read_temperature_centi() {
  constexpr int kSamples = 16;
  uint32_t sum = 0;

  for (int i = 0; i < kSamples; ++i) {
    ADC_InterTSSampInit();          // MUST be called every sample
    (void)ADC_ExcutSingleConver();  // discard first conversion
    sum += ADC_ExcutSingleConver(); // real sample
  }

  const int32_t adc = static_cast<int32_t>((sum + kSamples / 2) / kSamples);

  const uint32_t c25 = *reinterpret_cast<volatile uint32_t *>(ROM_CFG_TMP_25C);
  const int32_t adc25 = static_cast<int32_t>(c25 & 0xFFFF);

  int32_t t_ref = static_cast<int32_t>((c25 >> 16) & 0xFFFF);
  if (t_ref == 0)
    t_ref = 25;

  int32_t centi = t_ref * 100 + ((adc - adc25) * 1000) / 27;

  if (centi < INT16_MIN)
    centi = INT16_MIN;
  if (centi > INT16_MAX)
    centi = INT16_MAX;

  return static_cast<int16_t>(centi);
}

uint16_t temperature() {
  return static_cast<uint16_t>(read_temperature_centi());
}

} // namespace Ch58xPlatform
