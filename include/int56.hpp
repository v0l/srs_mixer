#pragma once

#include <stdint.h>

namespace ssr
{
    class uint56_t
    {
    private:
        uint32_t low;
        uint16_t mid;
        uint16_t high;
    };
} // namespace ssr