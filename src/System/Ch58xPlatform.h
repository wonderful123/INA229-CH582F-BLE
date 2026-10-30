#pragma once

#include <cstdint>

namespace Ch58xPlatform {

void init_system();
uint32_t random_seed();
uint32_t sleep_ticks(uint32_t ticks);

uint16_t temperature(); // centi-degrees Celsius

} // namespace Ch58xPlatform
