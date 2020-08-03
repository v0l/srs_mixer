#pragma once

#include <stdint.h>

namespace ssr
{
    /**
     * Byte alligned 16bit value
     */
    static constexpr uint16_t __u16(const char *data)
    {
        return (uint16_t)(data[1] & 0xFF) |
               ((uint16_t)(data[0] & 0xFF) << 8u);
    }

    /** 
     * Byte alligned 24bit value
     */
    static constexpr uint32_t __u24(const char *data)
    {
        return (uint32_t)(data[2] & 0xFF) |
               ((uint32_t)(data[1] & 0xFF) << 8u) |
               ((uint32_t)(data[0] & 0xFF) << 16u);
    }

    /**
     * Byte alligned 32bit value
     */
    static constexpr uint32_t __u32(const char *data)
    {
        return (uint32_t)(data[3] & 0xFF) |
               ((uint32_t)(data[2] & 0xFF) << 8u) |
               ((uint32_t)(data[1] & 0xFF) << 16u) |
               ((uint32_t)(data[0] & 0xFF) << 24u);
    }

    /**
     * Byte alligned 40bit value
     */
    static constexpr uint64_t __u40(const char *data)
    {
        return (uint64_t)(data[4] & 0xFF) |
               ((uint64_t)(data[3] & 0xFF) << 8u) |
               ((uint64_t)(data[2] & 0xFF) << 16u) |
               ((uint64_t)(data[1] & 0xFF) << 24u) |
               ((uint64_t)(data[0] & 0xFF) << 32u);
    }

    /**
     * Byte alligned 48bit value
     */
    static constexpr uint64_t __u48(const char *data)
    {
        return (uint64_t)(data[5] & 0xFF) |
               ((uint64_t)(data[4] & 0xFF) << 8u) |
               ((uint64_t)(data[3] & 0xFF) << 16u) |
               ((uint64_t)(data[2] & 0xFF) << 24u) |
               ((uint64_t)(data[1] & 0xFF) << 32u) |
               ((uint64_t)(data[0] & 0xFF) << 40u);
    }

    /**
     * Byte alligned 56bit value
     */
    static constexpr uint64_t __u56(const char *data)
    {
        return (uint64_t)(data[6] & 0xFF) |
               ((uint64_t)(data[5] & 0xFF) << 8u) |
               ((uint64_t)(data[4] & 0xFF) << 16u) |
               ((uint64_t)(data[3] & 0xFF) << 24u) |
               ((uint64_t)(data[2] & 0xFF) << 32u) |
               ((uint64_t)(data[1] & 0xFF) << 40u) |
               ((uint64_t)(data[0] & 0xFF) << 48u);
    }
} // namespace ssr
