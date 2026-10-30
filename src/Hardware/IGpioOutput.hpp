#pragma once

namespace Hardware {

class IGpioOutput {
public:
  virtual ~IGpioOutput() = default;

  virtual void init() = 0;
  virtual void setHigh() noexcept = 0;
  virtual void setLow() noexcept = 0;
};

} // namespace Hardware