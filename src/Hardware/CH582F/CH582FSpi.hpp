#pragma once

#include "Hardware/ISpiBus.hpp"
#include <cstddef>
#include <cstdint>

extern "C" {
#include "CH58x_common.h"
}

#undef min
#undef max

// Drastically slow down the clock for debugging (approx 500 kHz).
// This eliminates trace capacitance, RC rise-time limits, and slew-rate issues.
#ifndef SPI_HALF_PERIOD_NOPS
#define SPI_HALF_PERIOD_NOPS 40
#endif

namespace Hardware {

// Software (bit-banged) SPI master for INA229
// TI SPI Mode 1 (CPOL=0, CPHA=1): Idle low, Shift on Rising, Sample on Falling
class CH582FSpi final : public ISpiBus {
public:
  static constexpr uint32_t kSckPin = GPIO_Pin_13;
  static constexpr uint32_t kMisoPin = GPIO_Pin_14;
  static constexpr uint32_t kMosiPin = GPIO_Pin_15;

  CH582FSpi() = default;

  void init() {
    // SCLK idles low in SPI Mode 1.
    GPIOB_ResetBits(kSckPin | kMosiPin);
    GPIOB_ModeCfg(kSckPin | kMosiPin, GPIO_ModeOut_PP_5mA);
    GPIOB_ModeCfg(kMisoPin, GPIO_ModeIN_PU);
  }

  SpiStatus transfer(const uint8_t *tx, uint8_t *rx, size_t len) override {
    // Universal CS settling delay: guarantees CS has reached GND 
    // and the INA229 is ready before SCLK moves.
    for(int i = 0; i < 4; ++i) half_period_delay();

    for (size_t i = 0; i < len; ++i) {
      const uint8_t txByte = (tx != nullptr) ? tx[i] : 0xFF;
      const uint8_t rxByte = transferByte(txByte);

      if (rx != nullptr) {
        rx[i] = rxByte;
      }
    }

    // Universal CS hold delay: guarantees the final SCLK falling edge 
    // has completed before CS rises.
    for(int i = 0; i < 4; ++i) half_period_delay();

    return SpiStatus::Success;
  }

private:
  static void half_period_delay() {
    for (uint32_t i = 0; i < SPI_HALF_PERIOD_NOPS; ++i) {
      __asm__ volatile("nop");
    }
  }

  static uint8_t transferByte(uint8_t out) {
    uint8_t in = 0;

    for (int bit = 7; bit >= 0; --bit) {
      // 1. LEADING EDGE (Rising)
      GPIOB_SetBits(kSckPin);

      // In TI Mode 1, data is shifted OUT on the rising edge.
      if (out & (1u << bit)) {
        GPIOB_SetBits(kMosiPin);
      } else {
        GPIOB_ResetBits(kMosiPin);
      }

      // Hold SCLK high 
      half_period_delay();

      // 2. TRAILING EDGE (Falling)
      // The INA229 samples our MOSI on this edge.
      GPIOB_ResetBits(kSckPin);

      // Hold SCLK low to allow MISO to settle for sampling
      half_period_delay();

      // Master samples MISO 
      in = static_cast<uint8_t>(in << 1);
      if (GPIOB_ReadPortPin(kMisoPin) != 0) {
        in |= 1u;
      }
    }

    return in;
  }
};

} // namespace Hardware