// src/BLE/Ch58xBleAdvertiser.h
#pragma once

#include "IBleAdvertiser.h"
#include <chrono>
#include <cstddef>
#include <cstdint>

class Ch58xBleAdvertiser final : public IBleAdvertiser {
public:
  Ch58xBleAdvertiser() = default;
  ~Ch58xBleAdvertiser() override = default;

  Ch58xBleAdvertiser(const Ch58xBleAdvertiser &) = delete;
  Ch58xBleAdvertiser &operator=(const Ch58xBleAdvertiser &) = delete;
  Ch58xBleAdvertiser(Ch58xBleAdvertiser &&) = delete;
  Ch58xBleAdvertiser &operator=(Ch58xBleAdvertiser &&) = delete;

  BleResult init() override;
  BleResult set_payload(AdvPayloadType type, ByteSpan payload) override;
  BleResult set_interval(std::chrono::milliseconds interval) override;
  BleResult start() override;
  BleResult stop() override;

  [[nodiscard]] bool is_advertising() const noexcept override;

  static constexpr std::size_t kBleMemorySize = 10 * 1024;

private:
  static constexpr std::size_t kMaxLegacyAdvLen = 31;
  static constexpr std::chrono::microseconds kBleTick{625};
  static constexpr std::chrono::milliseconds kMinAdvInterval{20};
  static constexpr std::chrono::milliseconds kMaxAdvInterval{10240};

  [[nodiscard]] static constexpr uint16_t
  ms_to_ble_ticks(std::chrono::milliseconds interval) noexcept {
    const auto us =
        std::chrono::duration_cast<std::chrono::microseconds>(interval).count();
    return static_cast<uint16_t>(us / kBleTick.count());
  }

  [[nodiscard]] static constexpr bool
  is_valid_interval(std::chrono::milliseconds interval) noexcept {
    return interval >= kMinAdvInterval && interval <= kMaxAdvInterval;
  }

  [[nodiscard]] bool ready() const noexcept { return initialized_; }

  bool initialized_{false};
};