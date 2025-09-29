#include "observer.h"
#include "logger.h"
#include <iostream>

namespace ccsim::core {

  void LoggingTelemetryObserver::on_telemetry_received(const domain::TelemetryReport& report) {
    auto& logger = Logger::instance();
    logger.info("Telemetry received from unit: " + report.data().unit_id.value() + 
                " at position (" + std::to_string(report.data().position.x) + 
                ", " + std::to_string(report.data().position.y) + 
                ", " + std::to_string(report.data().position.z) + ")");
  }

  void LoggingCommandObserver::on_command_sent(const domain::Command& command) {
    auto& logger = Logger::instance();
    logger.info("Command sent: " + command.id() + " to unit: " + command.target_id().value());
  }

  void LoggingCommandObserver::on_command_completed(const std::string& command_id, bool success) {
    auto& logger = Logger::instance();
    logger.info("Command completed: " + command_id + " - " + (success ? "SUCCESS" : "FAILED"));
  }

} // namespace ccsim::core
