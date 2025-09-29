# Command & Control Simulation

A real-time command and control simulation system built in C++20 with multithreading and design patterns.

## Features

- **Controller Server**: Manages agent connections, dispatches commands, collects telemetry
- **Agent Clients**: Execute commands and report telemetry data
- **Real-time Communication**: TCP-based protocol using Boost.Asio
- **Design Patterns**: Observer, Strategy, Factory, Singleton
- **Performance**: Supports 200+ concurrent simulated units
- **Monitoring**: CLI interface for unit status and telemetry

## Architecture

```
command-control-sim/
├── src/
│   ├── controller/     # Server components
│   ├── agent/         # Client components  
│   ├── domain/        # Core models (Command, Telemetry, Unit)
│   ├── core/          # Design patterns (Observer, Strategy, etc.)
│   ├── net/           # Network layer (Boost.Asio)
│   └── cli/           # Command line interface
├── tests/             # Unit tests
└── docs/              # Documentation
```

## Building

### Prerequisites

- C++20 compatible compiler (GCC 10+, Clang 12+, MSVC 2019+)
- CMake 3.20+
- Boost 1.70+
- nlohmann/json
- GoogleTest (for tests)

### Build Commands

```bash
mkdir build && cd build
cmake ..
make -j$(nproc)

# Run tests
make test

# Run components
./controller    # Start the controller server
./agent         # Start an agent client
./cli           # Start the monitoring interface
```

## Usage

1. **Start Controller**: `./controller` (default port 8080)
2. **Start Agents**: `./agent --server localhost:8080 --id agent_001`
3. **Monitor**: `./cli --connect localhost:8080`

## Protocol

Commands and telemetry are exchanged using JSON over TCP:
- **MOVE**: Change agent position
- **REPORT**: Request status update
- **ALERT**: Emergency notification

## Performance Targets

- 200+ concurrent simulated units
- P95 response time < 150ms (local)
- 70%+ test coverage

## License

MIT License
