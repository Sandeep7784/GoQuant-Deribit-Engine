cmake_minimum_required(VERSION 3.12)
project(deribit_trading_system)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find required packages
find_package(cpprestsdk REQUIRED)
find_package(Threads REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(Boost REQUIRED COMPONENTS system)

# Add JsonCpp
find_package(PkgConfig REQUIRED)
pkg_check_modules(JSONCPP jsoncpp)

# Find websocketpp
find_package(websocketpp REQUIRED)

# Include directories
include_directories(
    ${CMAKE_SOURCE_DIR}/include
    ${JSONCPP_INCLUDE_DIRS}
    ${websocketpp_INCLUDE_DIRS}
)

# Source files
file(GLOB SOURCES "src/*.cpp")

# Create main executable
add_executable(${PROJECT_NAME} ${SOURCES})

# Create test client executable
add_executable(websocket_client_test tests/websocket_client_test.cpp)

# Link libraries for main executable
target_link_libraries(${PROJECT_NAME}
    PRIVATE
    cpprestsdk::cpprest
    ${CMAKE_THREAD_LIBS_INIT}
    OpenSSL::SSL
    OpenSSL::Crypto
    ${Boost_LIBRARIES}
    ${JSONCPP_LIBRARIES}
    websocketpp::websocketpp
)

# Link libraries for test client
target_link_libraries(websocket_client_test
    PRIVATE
    ${CMAKE_THREAD_LIBS_INIT}
    OpenSSL::SSL
    OpenSSL::Crypto
    ${Boost_LIBRARIES}
    ${JSONCPP_LIBRARIES}
    websocketpp::websocketpp
)

# Copy config file to build directory
configure_file(
    ${CMAKE_SOURCE_DIR}/config/config.json.example
    ${CMAKE_BINARY_DIR}/config/config.json.example
    COPYONLY
)