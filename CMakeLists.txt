cmake_minimum_required(VERSION 3.0.0)
project(ssr_mixer VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 17)

include(cmake/FindLibUV.cmake)

set(SSR_SRS
    src/modes.cpp
    src/ssr_mixer.cpp
)

add_executable(ssr_mixer ${SSR_SRS})
target_include_directories(ssr_mixer PUBLIC include)
target_link_libraries(ssr_mixer ${LIBUV_LIBRARIES})