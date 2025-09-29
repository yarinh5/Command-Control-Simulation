# Command & Control Simulation - Architecture

## Overview

This document describes the architecture of the Command & Control Simulation system, a real-time distributed system built in C++20 that demonstrates OOP principles, multithreading, and design patterns.

## System Architecture

```
┌─────────────────┐    ┌─────────────────┐    ┌─────────────────┐
│    Controller   │    │     Agent 1     │    │     Agent 2     │
│    (Server)     │◄──►│    (Client)     │    │    (Client)     │
└─────────────────┘    └─────────────────┘    └─────────────────┘
         │                       │                       │
         │                       │                       │
         ▼                       ▼                       ▼
┌─────────────────┐    ┌─────────────────┐    ┌─────────────────┐
│     CLI         │    │   Telemetry     │    │   Telemetry     │
│   Monitor       │    │   Reports       │    │   Reports       │
└─────────────────┘    └─────────────────┘    └─────────────────┘
```

## Core Components

### 1. Domain Layer (`src/domain/`)

**Purpose**: Core business logic and data models

**Components**:
- `UnitId`: Unique identifier for units with hash support
- `Unit`: Represents an agent with position, status, and timestamps
- `Command`: Base command class with JSON serialization
- `TelemetryData`: Telemetry information from units
- `CommandFactory`: Factory pattern for creating commands

**Key Features**:
- Immutable data structures where appropriate
- JSON serialization/deserialization
- Type-safe enums for status and command types

### 2. Core Patterns Layer (`src/core/`)

**Purpose**: Implementation of design patterns and core functionality

**Components**:
- `Observer`: Event notification system for telemetry and commands
- `Strategy`: Command dispatch strategies (Round-robin, Priority, Broadcast)
- `Logger`: Singleton logging system with performance timing
- `Registry`: Singleton unit registry for managing units

**Design Patterns**:
- **Observer Pattern**: Telemetry and command event notifications
- **Strategy Pattern**: Configurable command dispatch algorithms
- **Singleton Pattern**: Logger and Registry instances
- **Factory Pattern**: Command creation

### 3. Network Layer (`src/net/`)

**Purpose**: TCP communication using Boost.Asio

**Components**:
- `TcpServer`: Asynchronous TCP server for Controller
- `TcpClient`: TCP client for Agents
- `ProtocolHandler`: JSON-based message protocol

**Features**:
- Asynchronous I/O with Boost.Asio
- Message framing with length headers
- JSON protocol for commands and telemetry
- Connection management and error handling

### 4. Controller (`src/controller/`)

**Purpose**: Central command and control server

**Components**:
- `Controller`: Main server class
- `CommandDispatcher`: Manages command distribution

**Responsibilities**:
- Accept agent connections
- Dispatch commands using configurable strategies
- Collect telemetry data
- Maintain unit registry
- Provide system monitoring

### 5. Agent (`src/agent/`)

**Purpose**: Client that executes commands and reports telemetry

**Components**:
- `Agent`: Main client class
- `CommandExecutor`: Executes received commands

**Responsibilities**:
- Connect to controller
- Execute commands (MOVE, REPORT, ALERT)
- Generate and send telemetry data
- Simulate realistic behavior (battery drain, movement)

### 6. CLI Interface (`src/cli/`)

**Purpose**: Monitoring and control interface

**Components**:
- `Monitor`: Real-time display of system status
- Interactive command interface

**Features**:
- Real-time unit status display
- Command history tracking
- Telemetry visualization
- Interactive command sending

## Design Patterns Implementation

### 1. Observer Pattern

```cpp
// Telemetry events
class TelemetryObserver {
    virtual void on_telemetry_received(const TelemetryReport& report) = 0;
};

// Command events
class CommandObserver {
    virtual void on_command_sent(const Command& command) = 0;
    virtual void on_command_completed(const string& command_id, bool success) = 0;
};
```

**Benefits**:
- Loose coupling between components
- Easy addition of new observers (logging, monitoring, etc.)
- Event-driven architecture

### 2. Strategy Pattern

```cpp
class CommandDispatchStrategy {
    virtual vector<UnitId> select_targets(
        const vector<UnitId>& available_units,
        const Command& command) = 0;
};
```

**Implementations**:
- `RoundRobinStrategy`: Even distribution
- `PriorityStrategy`: Priority-based selection
- `BroadcastStrategy`: Send to all units

### 3. Singleton Pattern

```cpp
class Logger {
    static Logger& instance();
    // Thread-safe singleton implementation
};
```

**Usage**:
- Centralized logging
- Unit registry management
- Configuration storage

### 4. Factory Pattern

```cpp
class CommandFactory {
    static unique_ptr<Command> create_move_command(const UnitId& target, const Position& dest);
    static unique_ptr<Command> create_report_command(const UnitId& target);
    static unique_ptr<Command> create_alert_command(const UnitId& target, const string& message);
};
```

## Multithreading Architecture

### Thread Safety

1. **Atomic Operations**: Status flags, counters
2. **Mutex Protection**: Shared data structures
3. **Lock-free Design**: Where possible for performance

### Threading Model

```
Controller:
├── Main Thread (I/O, command processing)
├── TCP Server Thread (Boost.Asio)
├── Command Processing Thread
└── Telemetry Collection Thread

Agent:
├── Main Thread (I/O, command execution)
├── TCP Client Thread (Boost.Asio)
└── Telemetry Reporting Thread
```

## Performance Characteristics

### Scalability
- **Target**: 200+ concurrent units
- **Architecture**: Asynchronous I/O prevents thread explosion
- **Memory**: Efficient data structures with move semantics

### Response Time
- **Target**: P95 < 150ms for command response
- **Optimizations**: 
  - Lock-free operations where possible
  - Efficient JSON serialization
  - Minimal memory allocations

### Resource Usage
- **CPU**: Event-driven architecture minimizes CPU usage
- **Memory**: RAII and smart pointers prevent leaks
- **Network**: Efficient binary protocol with message framing

## Protocol Design

### Message Format
```
[4-byte length][JSON payload]
```

### Command Types
- `MOVE`: Change unit position
- `REPORT`: Request status update
- `ALERT`: Emergency notification
- `SHUTDOWN`: Graceful shutdown

### Telemetry Format
```json
{
  "type": "telemetry",
  "data": {
    "unit_id": "unit_001",
    "position": {"x": 100.0, "y": 200.0, "z": 300.0},
    "status": 0,
    "battery_level": 85.5,
    "cpu_usage": 45.2,
    "memory_usage": 60.8,
    "timestamp": 1234567890
  }
}
```

## Error Handling

### Network Errors
- Connection timeouts
- Message parsing failures
- Protocol violations

### Application Errors
- Invalid commands
- Unit not found
- Resource exhaustion

### Recovery Strategies
- Automatic reconnection
- Graceful degradation
- Error logging and monitoring

## Testing Strategy

### Unit Tests
- Domain model validation
- Design pattern correctness
- Edge case handling

### Integration Tests
- Controller-Agent communication
- Command execution workflows
- Telemetry collection

### Performance Tests
- Load testing with 200+ units
- Response time measurements
- Memory usage profiling

## Future Enhancements

1. **Persistence Layer**: Database storage for commands and telemetry
2. **Web Interface**: REST API and web-based monitoring
3. **Load Balancing**: Multiple controller instances
4. **Security**: Authentication and encryption
5. **Advanced Strategies**: Machine learning-based command dispatch

## Dependencies

- **C++20**: Modern C++ features
- **Boost.Asio**: Asynchronous I/O
- **nlohmann/json**: JSON handling
- **GoogleTest**: Unit testing
- **CMake**: Build system

## Build Instructions

```bash
mkdir build && cd build
cmake ..
make -j$(nproc)
```

## Running the System

1. **Start Controller**: `./controller [port]`
2. **Start Agents**: `./agent --server localhost:8080 --id agent_001`
3. **Monitor System**: `./cli`

This architecture provides a solid foundation for a scalable, maintainable command and control system while demonstrating key software engineering principles and design patterns.
