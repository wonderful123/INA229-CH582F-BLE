#pragma once

#include "PayloadBuilder.h"

#include <array>
#include <cstddef>
#include <cstdint>

namespace Hardware {
class PowerSensors;
}

namespace PowerPayload {

// Protocol 2 manufacturer-specific data, little-endian throughout:
//
//   header       company ID (2) | protocol version (1) | payload type (1)
//   3 channels   voltage mV u16 (0xFFFF = invalid)
//                current i16, 10 mA units
//                charge i32, coulombs
//                in the order: charging (solar), distribution (load),
//                main battery
//   trailer      sequence (1)
//
// The sequence number counts packets and wraps at 256. A receiver that sees
// it stop changing knows the readings are stale.
//
// Framing this into an advertisement (AD length and type) is the job of
// PayloadBuilder.

constexpr uint16_t kCompanyId = 0xFFFF;
constexpr uint8_t kProtocolVersion = 0x02;
constexpr uint8_t kPayloadTypePower = 0x01;
constexpr uint16_t kInvalidVoltage = 0xFFFF;

constexpr std::size_t kChannelCount = 3;
constexpr std::size_t kChannelSize =
    sizeof(uint16_t) + sizeof(int16_t) + sizeof(int32_t);
constexpr std::size_t kHeaderSize = sizeof(kCompanyId) +
                                    sizeof(kProtocolVersion) +
                                    sizeof(kPayloadTypePower);
constexpr std::size_t kSequenceSize = sizeof(uint8_t);

constexpr std::size_t kPayloadSize =
    kHeaderSize + kChannelCount * kChannelSize + kSequenceSize;

static_assert(PayloadBuilder::kAdStructureOverhead + kPayloadSize <=
                  PayloadBuilder::kMaxAdvertisingDataSize,
              "payload does not fit in a legacy advertisement");

using ManufacturerData = std::array<uint8_t, kPayloadSize>;

// Produces the advertisements, owning the sequence number so callers cannot
// forget to advance it.
class Encoder {
public:
  // Encodes the sensors' current readings as the next advertisement in the
  // sequence. The result is always ok(): the size is checked at compile time.
  [[nodiscard]] PayloadBuilder::Advertisement
  next(const Hardware::PowerSensors &sensors) noexcept;

private:
  uint8_t m_sequence = 0;
};

} // namespace PowerPayload