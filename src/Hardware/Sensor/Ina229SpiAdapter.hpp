#pragma once

#include "Hardware/IGpioOutput.hpp"
#include "Hardware/ISpiBus.hpp"

#include <INA229/ISPI.hpp>

#include <array>
#include <cstddef>
#include <cstdint>
#include <cstring>

namespace Hardware {

class Ina229SpiAdapter final : public INA229::ISPI {
public:
  Ina229SpiAdapter(ISpiBus &bus, IGpioOutput &csPin)
      : m_bus(bus), m_csPin(csPin) {}

  void read(uint8_t command, uint8_t *rx, size_t len) override {
    if (rx == nullptr || len == 0 || len > kMaxReadLength) {
      return;
    }

    std::array<uint8_t, kMaxReadTransferLength> txBuf{};
    std::array<uint8_t, kMaxReadTransferLength> rxBuf{};

    txBuf[0] = command;

    for (size_t i = 0; i < len; ++i) {
      txBuf[i + 1] = 0xFF;
    }

    {
      CsGuard guard(m_csPin);

      if (m_bus.transfer(txBuf.data(), rxBuf.data(), len + 1) !=
          SpiStatus::Success) {
        // Give the caller deterministic zeros rather than stale bytes.
        std::memset(rx, 0, len);
        return;
      }
    }

    std::memcpy(rx, rxBuf.data() + 1, len);
  }

  void write(uint8_t command, const uint8_t *tx, size_t len) override {
    if (tx == nullptr || len == 0 || len > kMaxWriteLength) {
      return;
    }

    std::array<uint8_t, kMaxWriteTransferLength> txBuf{};

    txBuf[0] = command;

    std::memcpy(txBuf.data() + 1, tx, len);

    CsGuard guard(m_csPin);

    (void)m_bus.transfer(txBuf.data(), nullptr, len + 1);
  }

private:
  static constexpr size_t kMaxReadLength = 5;
  static constexpr size_t kMaxReadTransferLength = kMaxReadLength + 1;

  static constexpr size_t kMaxWriteLength = 3;
  static constexpr size_t kMaxWriteTransferLength = kMaxWriteLength + 1;

  struct CsGuard {
    explicit CsGuard(IGpioOutput &pin) noexcept : pin(pin) { pin.setLow(); }

    ~CsGuard() { pin.setHigh(); }

    IGpioOutput &pin;
  };

  ISpiBus &m_bus;
  IGpioOutput &m_csPin;
};

} // namespace Hardware