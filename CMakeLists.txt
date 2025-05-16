cmake_minimum_required(VERSION 3.15)
project(trade_simulator)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Include paths
include_directories(include)
include_directories(third_party)

# Crow Dependencies
find_package(Boost REQUIRED system thread)
find_package(OpenSSL REQUIRED)

# Source files
file(GLOB_RECURSE SRC_FILES src/*.cpp)

# Main executable
add_executable(trade_simulator ${SRC_FILES})

# Link dependencies
target_link_libraries(trade_simulator
    Boost::system
    Boost::thread
    OpenSSL::SSL
    OpenSSL::Crypto
    pthread
)
