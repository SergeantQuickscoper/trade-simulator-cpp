cmake_minimum_required(VERSION 3.15)
project(trade_simulator)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Include paths
include_directories(include)
include_directories(thirdParty)

# Add Boost.Beast definitions
add_definitions(-DBOOST_BEAST_USE_STD_STRING_VIEW)
add_definitions(-DBOOST_ASIO_NO_DEPRECATED)

# Dependencies
find_package(Boost REQUIRED COMPONENTS system thread)
find_package(OpenSSL REQUIRED)

# Source files
file(GLOB_RECURSE SRC_FILES src/*.cpp)

# Main executable
add_executable(trade_simulator ${SRC_FILES})

# Link dependencies
target_link_libraries(trade_simulator
    PRIVATE
    Boost::system
    Boost::thread
    OpenSSL::SSL
    OpenSSL::Crypto
    pthread
    ws2_32
)

# Enable warnings
if(MSVC)
    target_compile_options(trade_simulator PRIVATE /W4)
else()
    target_compile_options(trade_simulator PRIVATE -Wall -Wextra -Wpedantic)
endif()

