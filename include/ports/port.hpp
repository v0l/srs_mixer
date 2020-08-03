#pragma once

#include <uvw.hpp>

#include <stdint.h>
#include <memory>

namespace ssr::ports
{
    class Port {
    public:
        Port(uint16_t port) : _port(port) {}
        
        virtual void Init(uvw::Loop &loop) = 0;

        template<class Tmsg>
        void Mix(Tmsg msg);

        protected:
            uint16_t _port;
            std::shared_ptr<uvw::TCPHandle> _tcp;
    };

} // namespace ssr::ports