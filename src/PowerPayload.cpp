// src/PowerPayload.cpp
#include "PowerPayload.h"

#include "Hardware/PowerSensors.hpp"

#include <cassert>
#include <cmath>
#include <cstdint>
#include <limits>
#include <optional>
#include <type_traits>

namespace PowerPayload {

namespace {

// Appends integers to the manufacturer data in little-endian byte order.
// Writes past the end are dropped, so a layout mistake cannot corrupt memory.
class Writer {
public:
  explicit Writer(ManufacturerData &out) noexcept : m_out(out) {}

  template <typename T> Writer &put(T value) noexcept {
    static_assert(std::is_integral_v<T>, "integers only");

    const auto raw = static_cast<std::make_unsigned_t<T>>(value);

    for (std::size_t i = 0; i < sizeof(T); ++i) {
      if (m_size < m_out.size()) {
        m_out[m_size++] = static_cast<uint8_t>(raw >> (8 * i));
      }
    }

    return *this;
  }

  // True once every byte of the manufacturer data has been written.
  [[nodiscard]] bool full() const noexcept { return m_size == m_out.size(); }

private:
  ManufacturerData &m_out;
  std::size_t m_size = 0;
};

// Rounds to the nearest integer, saturating at [lo, hi]. `value` must be
// finite.
template <typename T>
T to_int(double value, double lo = std::numeric_limits<T>::lowest(),
         double hi = std::numeric_limits<T>::max()) noexcept {
  return static_cast<T>(round(fmin(fmax(value, lo), hi)));
}

uint16_t encode_voltage(double volts) noexcept {
  // 0xFFFF is reserved for "invalid". Max valid reading is 65.534 V.
  return to_int<uint16_t>(volts * 1000.0, 0.0, kInvalidVoltage - 1.0);
}

int16_t encode_current(double amps) noexcept {
  // Clamp explicitly to int16 limits (-327.68 A to +327.67 A at 10 mA per LSB).
  return to_int<int16_t>(amps * 100.0, -32768.0, 32767.0);
}

int32_t encode_charge(double coulombs) noexcept {
  return to_int<int32_t>(coulombs);
}

// Checks if an optional measurement exists and contains a finite number.
bool is_usable(const std::optional<double> &value) noexcept {
  return value.has_value() && std::isfinite(*value);
}

// A channel is valid only if the chip is healthy and every required value is usable.
bool is_valid(const Sensor::SensorReading &reading) noexcept {
  return reading.isHealthy &&
         is_usable(reading.busVoltage) &&
         is_usable(reading.currentAmps) &&
         is_usable(reading.chargeCoulombs);
}

void write_channel(Writer &writer,
                   const Sensor::SensorReading &reading) noexcept {
  if (!is_valid(reading)) {
    writer.put(kInvalidVoltage).put(int16_t{0}).put(int32_t{0});
    return;
  }

  writer.put(encode_voltage(*reading.busVoltage))
      .put(encode_current(*reading.currentAmps))
      .put(encode_charge(*reading.chargeCoulombs));
}

// Encodes the readings and a sequence number as the protocol's
// manufacturer data.
ManufacturerData encode(const Hardware::PowerSensors &sensors,
                        uint8_t sequence) noexcept {
  ManufacturerData data{};
  Writer writer{data};

  writer.put(kCompanyId).put(kProtocolVersion).put(kPayloadTypePower);

  // Channel order is fixed by the protocol.
  write_channel(writer, sensors.solar().read());    // charging
  write_channel(writer, sensors.load().read());     // distribution
  write_channel(writer, sensors.battery().read()); // main battery

  writer.put(sequence);

  assert(writer.full());

  return data;
}

} // namespace

PayloadBuilder::Advertisement
Encoder::next(const Hardware::PowerSensors &sensors) noexcept {
  const ManufacturerData data = encode(sensors, m_sequence++);

  PayloadBuilder::Builder builder;
  builder.append(PayloadBuilder::AdType::kManufacturerSpecific, data);

  const PayloadBuilder::Advertisement advertisement = builder.finish();

  assert(advertisement.ok());

  return advertisement;
}

} // namespace PowerPayload