@echo off
REM Command & Control Simulation Build Script for Windows

echo === Command & Control Simulation Build Script ===

REM Check if we're in the right directory
if not exist "CMakeLists.txt" (
    echo Error: CMakeLists.txt not found. Please run this script from the project root.
    exit /b 1
)

REM Create build directory
echo Creating build directory...
if not exist "build" mkdir build
cd build

REM Configure with CMake
echo Configuring project with CMake...
cmake .. -DCMAKE_BUILD_TYPE=Release

REM Build the project
echo Building project...
cmake --build . --config Release

REM Run tests
echo Running tests...
ctest --output-on-failure

echo.
echo === Build Complete ===
echo Executables created:
echo   - Release\controller.exe    (Controller server)
echo   - Release\agent.exe         (Agent client)
echo   - Release\cli.exe           (CLI monitor)
echo   - Release\unit_tests.exe    (Unit tests)
echo.
echo Usage:
echo   1. Start controller: Release\controller.exe [port]
echo   2. Start agents: Release\agent.exe --server localhost:8080 --id agent_001
echo   3. Monitor system: Release\cli.exe
echo.
