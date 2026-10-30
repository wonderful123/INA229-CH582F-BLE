#pragma once

#include <cstddef>
#include <cstdint>

namespace Hardware {

enum class SpiStatus : uint8_t { Success, Timeout, Error };

class ISpiBus {
public:
  virtual ~ISpiBus() = default;

  /// @brief Full-duplex SPI transfer (simultaneous send and receive)
  [[nodiscard]] virtual SpiStatus transfer(const uint8_t *tx, uint8_t *rx,
                                           size_t len) = 0;
};

} // namespace Hardware