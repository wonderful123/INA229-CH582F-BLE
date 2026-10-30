// src/PayloadBuilder.cpp

#include "PayloadBuilder.h"

#include <cstring>

namespace PayloadBuilder {

Advertisement::Advertisement(const uint8_t *data, std::size_t size) noexcept
    : size_(size <= kMaxAdvertisingDataSize ? size : 0),
      ok_(size <= kMaxAdvertisingDataSize) {
  if (ok_ && size_ > 0) {
    std::memcpy(data_, data, size_);
  }
}

bool Builder::append(uint8_t type, const uint8_t *data,
                     std::size_t size) noexcept {
  if (!ok_) {
    return false;
  }

  constexpr std::size_t kStructureOverhead = 2;

  if (size > kMaxAdvertisingDataSize - kStructureOverhead ||
      kStructureOverhead + size > kMaxAdvertisingDataSize - size_ ||
      (size > 0 && data == nullptr)) {
    ok_ = false;
    return false;
  }

  data_[size_++] = static_cast<uint8_t>(size + 1);
  data_[size_++] = type;

  if (size > 0) {
    std::memcpy(data_ + size_, data, size);
    size_ += size;
  }

  return true;
}

Advertisement Builder::finish() const noexcept {
  return ok_ ? Advertisement(data_, size_) : Advertisement();
}

} // namespace PayloadBuilder