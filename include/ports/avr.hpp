#pragma once

#include <spdlog/spdlog.h>

#include <ports/port.hpp>
#include <ads-b/modes.hpp>

namespace ssr::ports
{
    class AVR : Port {
    public:
        AVR(uint16_t port) : Port(port) {

        }

        void Init(uvw::Loop &loop) {
            _tcp = loop.resource<uvw::TCPHandle>();

            _tcp->on<uvw::ErrorEvent>([](const uvw::ErrorEvent &err, uvw::TCPHandle &srv) {
                spdlog::error("Some error...");
            });
            _tcp->once<uvw::ListenEvent>([this](const uvw::ListenEvent &, uvw::TCPHandle &srv) {
                std::shared_ptr<uvw::TCPHandle> client = srv.loop().resource<uvw::TCPHandle>();

                client->on<uvw::CloseEvent>([](const uvw::CloseEvent &ev, uvw::TCPHandle &client) {
                    spdlog::warn("Unknown close event");
                    //ptr->close(); 
                });
                client->on<uvw::EndEvent>([](const uvw::EndEvent &, uvw::TCPHandle &client) {
                    spdlog::debug("Client disconnected {}:{}", client.peer().ip, client.peer().port);
                    client.close();
                });
                client->on<uvw::DataEvent>([this](const uvw::DataEvent &event, uvw::TCPHandle &client) {
                    this->ParseData(event);
                });

                srv.accept(*client);
                client->read();
                spdlog::debug("New client connected [{}] << {}:{}", this->_port, client->peer().ip, client->peer().port);
            });

            _tcp->bind("0.0.0.0", _port);
            _tcp->listen();
            spdlog::debug("AVR[in] started on {0}", _port);
        }

    private:
        void ParseData(const uvw::DataEvent &ev) {
            auto str = std::string(ev.data.get(), ev.data.get() + ev.length);
            std::string::size_type pos = 0;
            std::string::size_type prev = 0;

            while ((pos = str.find("\n", prev)) != std::string::npos)
            {
                ParseLine(str.substr(prev, pos - prev));
                prev = pos + 1;
            }
            
        }

        void ParseLine(const std::string &line) {
            if(line[0] == '@' || line[0] == '*') {
                auto msg = _modes.decodeHexMessage(line);
                auto newMsg = (ssr::ads_b::transport::ModeSNew*)msg->msg;
                spdlog::debug("Got Mode-S message [{}][{}]: {},{}", msg->crcok ? "OK " : "ERR", msg->errorbit, msg->metype, msg->mesub);
            }
        }

        ssr::ads_b::transport::ModeS _modes;
        char _buffer[1024]; //internal buffer for incomplete messages
        uint16_t _offset, _len;
    };

} // namespace ssr::ports
