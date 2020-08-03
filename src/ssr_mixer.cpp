#include <iostream>

#include <uvw.hpp>
#include <cxxopts.hpp>

#include <ports/avr.hpp>

int main(int argc, char** argv) {
    cxxopts::Options options("ssr_mixer", "SSR Mixer service");

    options.add_options()

        ("v,verbose", "Verbose output")
        ("b,bar", "Param bar", cxxopts::value<std::string>())
        ("d,debug", "Enable debugging", cxxopts::value<bool>()->default_value("false"))
        ("f,foo", "Param foo", cxxopts::value<int>()->default_value("10"))
        ("h,help", "Print usage")
    ;

    auto result = options.parse(argc, argv);
    if (result.count("help"))
    {
      std::cout << options.help() << std::endl;
      exit(0);
    }

    if(result.count("verbose")) {
        spdlog::set_level(spdlog::level::debug);
    }

    spdlog::info("ssr_mixer is starting!");

    auto loop = uvw::Loop::getDefault();

    auto avrIn = new ssr::ports::AVR(40002);
    avrIn->Init(*loop);

    loop->run();

    spdlog::info("Bye :)");
    return 0;
}
