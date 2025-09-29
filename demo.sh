#!/bin/bash

# Command & Control Simulation Demo Script

set -e

echo "=== Command & Control Simulation Demo ==="

# Check if executables exist
if [ ! -f "build/controller" ] || [ ! -f "build/agent" ] || [ ! -f "build/cli" ]; then
    echo "Error: Executables not found. Please run build.sh first."
    exit 1
fi

cd build

echo "Starting Controller server on port 8080..."
./controller 8080 &
CONTROLLER_PID=$!

# Wait for controller to start
sleep 2

echo "Starting 3 agents..."
./agent --server localhost:8080 --id agent_001 &
AGENT1_PID=$!

./agent --server localhost:8080 --id agent_002 &
AGENT2_PID=$!

./agent --server localhost:8080 --id agent_003 &
AGENT3_PID=$!

# Wait for agents to connect
sleep 3

echo ""
echo "=== Demo Running ==="
echo "Controller PID: $CONTROLLER_PID"
echo "Agent PIDs: $AGENT1_PID, $AGENT2_PID, $AGENT3_PID"
echo ""
echo "You can now:"
echo "1. Run './cli' in another terminal to monitor the system"
echo "2. The agents will automatically report telemetry every 5 seconds"
echo "3. The controller will collect telemetry and manage commands"
echo ""
echo "Press Ctrl+C to stop the demo..."

# Function to cleanup on exit
cleanup() {
    echo ""
    echo "Stopping demo..."
    kill $CONTROLLER_PID 2>/dev/null || true
    kill $AGENT1_PID 2>/dev/null || true
    kill $AGENT2_PID 2>/dev/null || true
    kill $AGENT3_PID 2>/dev/null || true
    echo "Demo stopped."
}

# Set trap to cleanup on script exit
trap cleanup EXIT INT TERM

# Wait for user to stop
wait
