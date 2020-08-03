#pragma once

#include <ads-b/registers.hpp>
#include <ads-b/1090es.hpp>

namespace ssr::ads_b
{
    class System {
    private:
        Registers _reg;

    public:
        auto ES_AirbornePosition() const -> es::AirbornePosition {
            es::AirbornePosition ret;
            auto reg = Registers::BDS(0, 5);

            auto data = _reg.Value(Registers::ES_AirbornePosition);

            return ret;
        }
    };
} // namespace ssr::ads_b
