#pragma once

#include <cstdint>

#include "Hardware/IGpioOutput.hpp"

extern "C" {
#include "CH58x_common.h"
}

#undef min
#undef max

namespace Hardware {

enum class CH582Port : uint8_t {
  PortA,
  PortB
};

template <CH582Port Port>
class CH582FGpioOutput final : public IGpioOutput {
public:
  explicit CH582FGpioOutput(uint32_t pin) noexcept
      : m_pin(pin) {}

  void init() override {
    setHigh();

    if constexpr (Port == CH582Port::PortA) {
      GPIOA_ModeCfg(m_pin, GPIO_ModeOut_PP_5mA);
    } else {
      GPIOB_ModeCfg(m_pin, GPIO_ModeOut_PP_5mA);
    }
  }

  void setHigh() noexcept override {
    if constexpr (Port == CH582Port::PortA) {
      GPIOA_SetBits(m_pin);
    } else {
      GPIOB_SetBits(m_pin);
    }
  }

  void setLow() noexcept override {
    if constexpr (Port == CH582Port::PortA) {
      GPIOA_ResetBits(m_pin);
    } else {
      GPIOB_ResetBits(m_pin);
    }
  }

private:
  uint32_t m_pin;
};

} // namespace Hardware