#!/bin/bash

# Command & Control Simulation Build Script

set -e  # Exit on any error

echo "=== Command & Control Simulation Build Script ==="

# Check if we're in the right directory
if [ ! -f "CMakeLists.txt" ]; then
    echo "Error: CMakeLists.txt not found. Please run this script from the project root."
    exit 1
fi

# Create build directory
echo "Creating build directory..."
mkdir -p build
cd build

# Configure with CMake
echo "Configuring project with CMake..."
cmake .. -DCMAKE_BUILD_TYPE=Release

# Build the project
echo "Building project..."
make -j$(nproc)

# Run tests
echo "Running tests..."
make test

echo ""
echo "=== Build Complete ==="
echo "Executables created:"
echo "  - ./controller    (Controller server)"
echo "  - ./agent         (Agent client)"
echo "  - ./cli           (CLI monitor)"
echo "  - ./unit_tests    (Unit tests)"
echo ""
echo "Usage:"
echo "  1. Start controller: ./controller [port]"
echo "  2. Start agents: ./agent --server localhost:8080 --id agent_001"
echo "  3. Monitor system: ./cli"
echo ""
