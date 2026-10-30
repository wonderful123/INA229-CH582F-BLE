// src/PayloadBuilder.h
#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

namespace PayloadBuilder {

constexpr std::size_t kMaxAdvertisingDataSize = 31;

// Every AD structure costs a length byte and a type byte on top of its data.
constexpr std::size_t kAdStructureOverhead = 2;

// Common AD types (Bluetooth Assigned Numbers). Named here for readability
// only: the builder does not interpret them.
namespace AdType {
constexpr uint8_t kFlags = 0x01;
constexpr uint8_t kTxPowerLevel = 0x0A;
constexpr uint8_t kManufacturerSpecific = 0xFF;
} // namespace AdType

// Owns a completed legacy BLE advertising packet.
//
// The packet is immutable after construction.
// A failed Advertisement has size() == 0 and ok() == false.
class Advertisement {
public:
  Advertisement() = default;

  const uint8_t *data() const noexcept { return data_; }
  std::size_t size() const noexcept { return size_; }
  bool ok() const noexcept { return ok_; }

private:
  friend class Builder;

  Advertisement(const uint8_t *data, std::size_t size) noexcept;

  uint8_t data_[kMaxAdvertisingDataSize] = {};
  std::size_t size_ = 0;
  bool ok_ = false;
};

// Builds a legacy BLE advertising packet from arbitrary AD structures.
//
// append() accepts an AD type and its associated data.
// The BLE AD length byte is generated automatically.
//
// The builder does not interpret the AD type or payload.
class Builder {
public:
  Builder() = default;

  // Appends one AD structure.
  //
  // 'type' is the BLE AD type.
  // 'data' points to the AD data bytes.
  // 'size' is the number of AD data bytes.
  //
  // Returns false if the structure does not fit within the
  // 31-byte legacy advertising limit or if the builder has failed.
  bool append(uint8_t type, const uint8_t *data, std::size_t size) noexcept;

  // Convenience overload for fixed-size data.
  template <std::size_t N>
  bool append(uint8_t type, const std::array<uint8_t, N> &data) noexcept {
    return append(type, data.data(), data.size());
  }

  // Returns the completed advertisement.
  //
  // On failure, returns an invalid Advertisement.
  Advertisement finish() const noexcept;

  bool ok() const noexcept { return ok_; }
  std::size_t size() const noexcept { return size_; }

private:
  uint8_t data_[kMaxAdvertisingDataSize] = {};
  std::size_t size_ = 0;
  bool ok_ = true;
};

} // namespace PayloadBuilder