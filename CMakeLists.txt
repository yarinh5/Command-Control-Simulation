cmake_minimum_required(VERSION 3.20)
project(CommandControlSim VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Compiler flags
if(MSVC)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
else()
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic")
endif()

# Find required packages
find_package(Boost REQUIRED COMPONENTS system thread)

# Try to find nlohmann_json, if not found, use FetchContent
find_package(nlohmann_json QUIET)
if(NOT nlohmann_json_FOUND)
  include(FetchContent)
  FetchContent_Declare(
    nlohmann_json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG v3.11.2
  )
  FetchContent_MakeAvailable(nlohmann_json)
endif()

# Try to find GTest, if not found, use FetchContent
find_package(GTest QUIET)
if(NOT GTest_FOUND)
  include(FetchContent)
  FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG v1.14.0
  )
  FetchContent_MakeAvailable(googletest)
endif()

# Include directories
include_directories(src)

# Source files
set(CONTROLLER_SOURCES
  src/controller/controller.cpp
  src/controller/command_dispatcher.cpp
  src/domain/unit.cpp
  src/domain/command.cpp
  src/domain/telemetry.cpp
  src/core/observer.cpp
  src/core/strategy.cpp
  src/core/logger.cpp
  src/core/registry.cpp
  src/net/tcp_server.cpp
  src/net/protocol_handler.cpp
)

set(AGENT_SOURCES
  src/agent/agent.cpp
  src/agent/command_executor.cpp
  src/domain/unit.cpp
  src/domain/command.cpp
  src/domain/telemetry.cpp
  src/core/logger.cpp
  src/net/tcp_client.cpp
  src/net/protocol_handler.cpp
)

set(CLI_SOURCES
  src/cli/main.cpp
  src/cli/monitor.cpp
  src/domain/unit.cpp
  src/domain/command.cpp
  src/domain/telemetry.cpp
  src/core/observer.cpp
  src/core/logger.cpp
)

# Create executables
add_executable(controller ${CONTROLLER_SOURCES})
target_link_libraries(controller 
  Boost::system 
  Boost::thread 
  nlohmann_json::nlohmann_json
  pthread
)

add_executable(agent ${AGENT_SOURCES})
target_link_libraries(agent 
  Boost::system 
  Boost::thread 
  nlohmann_json::nlohmann_json
  pthread
)

add_executable(cli ${CLI_SOURCES})
target_link_libraries(cli 
  Boost::system 
  Boost::thread 
  nlohmann_json::nlohmann_json
  pthread
)

# Tests
enable_testing()

set(TEST_SOURCES
  tests/test_domain.cpp
  tests/test_observer.cpp
  tests/test_strategy.cpp
  tests/test_controller.cpp
  tests/test_agent.cpp
)

add_executable(unit_tests ${TEST_SOURCES})
target_link_libraries(unit_tests 
  GTest::gtest 
  GTest::gtest_main
  Boost::system 
  Boost::thread 
  nlohmann_json::nlohmann_json
  pthread
)

add_test(NAME unit_tests COMMAND unit_tests)
