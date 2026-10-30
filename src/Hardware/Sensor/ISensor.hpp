#pragma once

#include <optional>

namespace Sensor {

struct SensorReading {
  std::optional<double> currentAmps;
  std::optional<double> busVoltage;
  std::optional<double> chargeCoulombs;
  std::optional<double> energyJoules;
  bool isHealthy{false};
};

class ISensor {
public:
  virtual ~ISensor() = default;

  [[nodiscard]] virtual SensorReading read() const noexcept = 0;
};

} // namespace Sensor