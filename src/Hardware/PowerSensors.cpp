#include "Hardware/PowerSensors.hpp"
#include <array>
#include <cstddef>

namespace Hardware {

namespace {

constexpr INA229::DriverConfig SOLAR_CONFIG{
    .shuntResistor = 0.00025,
    .maxExpectedCurrent = 150.0,
    .mode = INA229::AdcMode::CONTINUOUS_ALL,
    .avg = INA229::AvgCount::AVG_256,
    .vBusTime = INA229::ConversionTime::TIME_1052us,
    .vShuntTime = INA229::ConversionTime::TIME_1052us,
    .tempTime = INA229::ConversionTime::TIME_1052us,
    .range = INA229::ShuntRange::RANGE_41mV}; // 4x resolution for 37.5 mV max drop

constexpr INA229::DriverConfig BATTERY_CONFIG{
    .shuntResistor = 0.00025,
    .maxExpectedCurrent = 300.0,
    .mode = INA229::AdcMode::CONTINUOUS_ALL,
    .avg = INA229::AvgCount::AVG_256,
    .vBusTime = INA229::ConversionTime::TIME_1052us,
    .vShuntTime = INA229::ConversionTime::TIME_1052us,
    .tempTime = INA229::ConversionTime::TIME_1052us,
    .range = INA229::ShuntRange::RANGE_164mV}; // Retains 300 A head room

constexpr INA229::DriverConfig LOAD_CONFIG{
    .shuntResistor = 0.00025,
    .maxExpectedCurrent = 300.0,
    .mode = INA229::AdcMode::CONTINUOUS_ALL,
    .avg = INA229::AvgCount::AVG_16,          // Reduced averaging for faster fault detection
    .vBusTime = INA229::ConversionTime::TIME_1052us,
    .vShuntTime = INA229::ConversionTime::TIME_1052us,
    .tempTime = INA229::ConversionTime::TIME_1052us,
    .range = INA229::ShuntRange::RANGE_164mV}; // Retains 300 A fault headroom
} // namespace

PowerSensors::PowerSensors(const PowerBuses &buses)
    : m_solarDriver(buses.solar), m_batteryDriver(buses.battery),
      m_loadDriver(buses.load), m_solarSensor(m_solarDriver),
      m_batterySensor(m_batteryDriver), m_loadSensor(m_loadDriver) {}

bool PowerSensors::init() {
  const struct {
    INA229::INA229_Driver *driver;
    const INA229::DriverConfig *config;
  } targets[] = {
      {&m_solarDriver, &SOLAR_CONFIG},
      {&m_batteryDriver, &BATTERY_CONFIG},
      {&m_loadDriver, &LOAD_CONFIG},
  };

  bool allConfigured = true;

  for (std::size_t i = 0; i < m_configured.size(); ++i) {
    if (!m_configured[i]) {
      m_configured[i] =
          (targets[i].driver->begin(*targets[i].config) == INA229::Status::OK);
    }

    allConfigured = allConfigured && m_configured[i];
  }

  return allConfigured;
}

} // namespace Hardware