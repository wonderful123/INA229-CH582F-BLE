#pragma once

#include <array>
#include <chrono>
#include <cstddef>
#include <cstdint>

struct ByteSpan {
  const uint8_t *ptr{nullptr};
  std::size_t len{0};

  constexpr ByteSpan() noexcept = default;

  constexpr ByteSpan(const uint8_t *data, std::size_t size) noexcept
      : ptr(data), len(size) {}

  template <std::size_t N>
  constexpr ByteSpan(const std::array<uint8_t, N> &data) noexcept
      : ptr(data.data()), len(N) {}

  [[nodiscard]] constexpr const uint8_t *data() const noexcept { return ptr; }

  [[nodiscard]] constexpr std::size_t size() const noexcept { return len; }
};

enum class BleResult : uint8_t {
  Success,
  NotInitialized,
  InvalidState,
  PayloadTooLarge,
  InvalidInterval,
  InvalidParam,
  HardwareError
};

enum class AdvPayloadType : uint8_t { AdvertisingData, ScanResponseData };

class IBleAdvertiser {
public:
  virtual ~IBleAdvertiser() = default;

  virtual BleResult init() = 0;

  virtual BleResult set_payload(AdvPayloadType type,
                                              ByteSpan payload) = 0;

  [[nodiscard]] virtual BleResult
  set_interval(std::chrono::milliseconds interval) = 0;

  virtual BleResult start() = 0;

  virtual BleResult stop() = 0;

  [[nodiscard]] virtual bool is_advertising() const noexcept = 0;
};