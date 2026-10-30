#pragma once

#include "Hardware/Sensor/INA229SensorAdapter.hpp"
#include "Hardware/Sensor/ISensor.hpp"
#include "Hardware/Sensor/Ina229SpiAdapter.hpp"
#include <INA229/INA229.hpp>
#include <array>

namespace Hardware {

struct PowerBuses {
  Ina229SpiAdapter &solar;
  Ina229SpiAdapter &battery;
  Ina229SpiAdapter &load;
};

class PowerSensors {
public:
  explicit PowerSensors(const PowerBuses &buses);

  // Configures every chip that is not configured yet and returns true once
  // all three are. Safe to call repeatedly: a chip that is already configured
  // is left alone (calling begin() again would reset its accumulators).
  bool init();

  [[nodiscard]] Sensor::ISensor &solar() noexcept { return m_solarSensor; }
  [[nodiscard]] Sensor::ISensor &battery() noexcept { return m_batterySensor; }
  [[nodiscard]] Sensor::ISensor &load() noexcept { return m_loadSensor; }

  [[nodiscard]] const Sensor::ISensor &solar() const noexcept {
    return m_solarSensor;
  }
  [[nodiscard]] const Sensor::ISensor &battery() const noexcept {
    return m_batterySensor;
  }
  [[nodiscard]] const Sensor::ISensor &load() const noexcept {
    return m_loadSensor;
  }

private:
  // 1. Hardware drivers wrapping the SPI adapters
  INA229::INA229_Driver m_solarDriver;
  INA229::INA229_Driver m_batteryDriver;
  INA229::INA229_Driver m_loadDriver;

  // 2. Sensor abstraction wrapping the hardware drivers
  Sensor::INA229SensorAdapter m_solarSensor;
  Sensor::INA229SensorAdapter m_batterySensor;
  Sensor::INA229SensorAdapter m_loadSensor;

  // Which chips have been configured successfully (solar, battery, load).
  std::array<bool, 3> m_configured{};
};

} // namespace Hardware