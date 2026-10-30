// src/Hardware/Sensor/INA229SensorAdapter.hpp
#pragma once

#include "Hardware/Sensor/ISensor.hpp"
#include <INA229/INA229.hpp>

namespace Sensor {

class INA229SensorAdapter final : public ISensor {
public:
  explicit INA229SensorAdapter(INA229::INA229_Driver &driver) noexcept
      : m_driver(driver) {}

  [[nodiscard]] SensorReading read() const noexcept override {
    SensorReading reading{};
    reading.isHealthy = m_driver.isHealthy();

    if (reading.isHealthy) {
      reading.currentAmps   = m_driver.getCurrentAmps();
      reading.busVoltage    = m_driver.getBusVoltage();
      reading.chargeCoulombs = m_driver.getChargeCoulombs();
      reading.energyJoules  = m_driver.getEnergyJoules();
    }

    return reading;
  }

private:
  INA229::INA229_Driver &m_driver;
};

} // namespace Sensor