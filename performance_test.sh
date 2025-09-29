#!/bin/bash

# Performance Test Script for Command & Control Simulation

set -e

echo "=== Performance Test Script ==="

# Check if executables exist
if [ ! -f "build/controller" ] || [ ! -f "build/agent" ]; then
    echo "Error: Executables not found. Please run build.sh first."
    exit 1
fi

cd build

# Test parameters
NUM_AGENTS=50
TEST_DURATION=60  # seconds
CONTROLLER_PORT=8080

echo "Starting performance test with $NUM_AGENTS agents for $TEST_DURATION seconds..."

# Start controller
echo "Starting Controller..."
./controller $CONTROLLER_PORT > controller.log 2>&1 &
CONTROLLER_PID=$!

# Wait for controller to start
sleep 3

# Start multiple agents
echo "Starting $NUM_AGENTS agents..."
AGENT_PIDS=()

for i in $(seq 1 $NUM_AGENTS); do
    ./agent --server localhost:$CONTROLLER_PORT --id "perf_agent_$i" > "agent_$i.log" 2>&1 &
    AGENT_PIDS+=($!)
done

echo "All agents started. Running test for $TEST_DURATION seconds..."

# Let the system run
sleep $TEST_DURATION

echo "Test completed. Collecting results..."

# Stop all processes
kill $CONTROLLER_PID 2>/dev/null || true
for pid in "${AGENT_PIDS[@]}"; do
    kill $pid 2>/dev/null || true
done

# Wait for processes to stop
sleep 2

# Analyze results
echo ""
echo "=== Performance Test Results ==="

# Count successful connections
SUCCESSFUL_AGENTS=0
for i in $(seq 1 $NUM_AGENTS); do
    if grep -q "connected successfully" "agent_$i.log"; then
        ((SUCCESSFUL_AGENTS++))
    fi
done

echo "Successful agent connections: $SUCCESSFUL_AGENTS / $NUM_AGENTS"

# Count telemetry reports
TOTAL_TELEMETRY=$(grep -c "Telemetry received" controller.log 2>/dev/null || echo "0")
echo "Total telemetry reports received: $TOTAL_TELEMETRY"

# Calculate telemetry rate
if [ $TEST_DURATION -gt 0 ]; then
    TELEMETRY_RATE=$((TOTAL_TELEMETRY / TEST_DURATION))
    echo "Telemetry rate: $TELEMETRY_RATE reports/second"
fi

# Check for errors
ERRORS=$(grep -c "ERROR" controller.log 2>/dev/null || echo "0")
echo "Controller errors: $ERRORS"

echo ""
echo "Performance test completed!"
echo "Log files:"
echo "  - controller.log"
echo "  - agent_*.log"

# Cleanup function
cleanup() {
    echo "Cleaning up..."
    kill $CONTROLLER_PID 2>/dev/null || true
    for pid in "${AGENT_PIDS[@]}"; do
        kill $pid 2>/dev/null || true
    done
}

# Set trap to cleanup on script exit
trap cleanup EXIT INT TERM
