#pragma once

#include "unit.h"
#include <nlohmann/json.hpp>
#include <string>
#include <memory>
#include <chrono>

namespace ccsim::domain {

  enum class CommandType {
    MOVE,
    REPORT,
    ALERT,
    SHUTDOWN
  };

  class Command {
  public:
    Command(const UnitId& target_id, CommandType type, const nlohmann::json& payload = {})
      : target_id_(target_id), type_(type), payload_(payload),
        timestamp_(std::chrono::steady_clock::now()),
        id_(generate_id()) {}
    
    const std::string& id() const { return id_; }
    const UnitId& target_id() const { return target_id_; }
    CommandType type() const { return type_; }
    const nlohmann::json& payload() const { return payload_; }
    std::chrono::steady_clock::time_point timestamp() const { return timestamp_; }
    
    nlohmann::json to_json() const {
      nlohmann::json j;
      j["id"] = id_;
      j["target_id"] = target_id_.value();
      j["type"] = static_cast<int>(type_);
      j["payload"] = payload_;
      j["timestamp"] = std::chrono::duration_cast<std::chrono::milliseconds>(
        timestamp_.time_since_epoch()).count();
      return j;
    }
    
    static std::unique_ptr<Command> from_json(const nlohmann::json& j) {
      if (!j.contains("target_id") || !j.contains("type")) {
        return nullptr;
      }
      
      auto target_id = UnitId(j["target_id"]);
      auto type = static_cast<CommandType>(j["type"]);
      auto payload = j.value("payload", nlohmann::json{});
      
      return std::make_unique<Command>(target_id, type, payload);
    }
    
  private:
    std::string generate_id() {
      static std::atomic<uint64_t> counter{0};
      return "cmd_" + std::to_string(counter.fetch_add(1));
    }
    
    std::string id_;
    UnitId target_id_;
    CommandType type_;
    nlohmann::json payload_;
    std::chrono::steady_clock::time_point timestamp_;
  };

  class MoveCommand : public Command {
  public:
    MoveCommand(const UnitId& target_id, const Position& destination)
      : Command(target_id, CommandType::MOVE, nlohmann::json{
          {"destination", {{"x", destination.x}, {"y", destination.y}, {"z", destination.z}}}
        }) {}
    
    Position destination() const {
      auto dest = payload_["destination"];
      return {dest["x"], dest["y"], dest["z"]};
    }
  };

  class ReportCommand : public Command {
  public:
    explicit ReportCommand(const UnitId& target_id)
      : Command(target_id, CommandType::REPORT) {}
  };

  class AlertCommand : public Command {
  public:
    AlertCommand(const UnitId& target_id, const std::string& message, int severity = 1)
      : Command(target_id, CommandType::ALERT, nlohmann::json{
          {"message", message}, {"severity", severity}
        }) {}
    
    std::string message() const { return payload_["message"]; }
    int severity() const { return payload_["severity"]; }
  };

  // Factory for creating commands
  class CommandFactory {
  public:
    static std::unique_ptr<Command> create_move_command(const UnitId& target_id, const Position& destination) {
      return std::make_unique<MoveCommand>(target_id, destination);
    }
    
    static std::unique_ptr<Command> create_report_command(const UnitId& target_id) {
      return std::make_unique<ReportCommand>(target_id);
    }
    
    static std::unique_ptr<Command> create_alert_command(const UnitId& target_id, const std::string& message, int severity = 1) {
      return std::make_unique<AlertCommand>(target_id, message, severity);
    }
    
    static std::unique_ptr<Command> create_from_json(const nlohmann::json& j) {
      return Command::from_json(j);
    }
  };

} // namespace ccsim::domain
