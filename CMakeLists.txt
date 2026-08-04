cmake_minimum_required(VERSION 3.15)
project(Web-PTT LANGUAGES CXX)

add_compile_definitions(NOMINMAX WIN32_LEAN_AND_MEAN)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

set(BOOST_INCLUDE_LIBRARIES asio beast uuid system CACHE STRING "" FORCE)
set(BOOST_ENABLE_CMAKE ON CACHE BOOL "" FORCE)
set(ENABLE_WARNINGS_AS_ERRORS OFF CACHE BOOL "" FORCE)


FetchContent_Declare(
    boost
    URL https://github.com/boostorg/boost/releases/download/boost-1.91.0-1/boost-1.91.0-1-cmake.zip
)

FetchContent_Declare(
    libdatachannel
    GIT_REPOSITORY https://github.com/paullouisageneau/libdatachannel.git
    GIT_TAG v0.24.5
)

FetchContent_Declare(
    spdlog
    URL https://github.com/gabime/spdlog/archive/refs/tags/v1.17.0.zip
)

FetchContent_Declare(
    glaze
    URL https://github.com/stephenberry/glaze/archive/refs/tags/v7.9.1.zip
)


FetchContent_MakeAvailable(boost libdatachannel spdlog glaze)

add_executable(backend "src/main.cpp" "src/api/Listener.cpp" "src/api/Session.cpp")
target_link_libraries(backend PRIVATE 
    Boost::asio
    Boost::beast
    datachannel
    glaze::glaze
    spdlog::spdlog)
