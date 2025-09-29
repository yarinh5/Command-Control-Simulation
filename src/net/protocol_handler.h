#pragma once

#include "domain/command.h"
#include "domain/telemetry.h"
#include <nlohmann/json.hpp>
#include <string>
#include <memory>
#include <functional>

namespace ccsim::net {

  // Protocol message types
  enum class MessageType {
    COMMAND,
    TELEMETRY,
    ACKNOWLEDGMENT,
    ERROR
  };

  // Protocol message wrapper
  struct ProtocolMessage {
    MessageType type;
    std::string payload;
    
    nlohmann::json to_json() const {
      nlohmann::json j;
      j["type"] = static_cast<int>(type);
      j["payload"] = payload;
      return j;
    }
    
    static std::unique_ptr<ProtocolMessage> from_json(const nlohmann::json& j) {
      if (!j.contains("type") || !j.contains("payload")) {
        return nullptr;
      }
      
      auto msg = std::make_unique<ProtocolMessage>();
      msg->type = static_cast<MessageType>(j["type"]);
      msg->payload = j["payload"];
      return msg;
    }
  };

  // Protocol handler interface
  class ProtocolHandler {
  public:
    virtual ~ProtocolHandler() = default;
    
    // Serialization
    virtual std::string serialize_command(const domain::Command& command) = 0;
    virtual std::string serialize_telemetry(const domain::TelemetryReport& report) = 0;
    virtual std::string serialize_message(const ProtocolMessage& message) = 0;
    
    // Deserialization
    virtual std::unique_ptr<domain::Command> deserialize_command(const std::string& data) = 0;
    virtual std::unique_ptr<domain::TelemetryReport> deserialize_telemetry(const std::string& data) = 0;
    virtual std::unique_ptr<ProtocolMessage> deserialize_message(const std::string& data) = 0;
    
    // Message creation helpers
    virtual ProtocolMessage create_command_message(const domain::Command& command) = 0;
    virtual ProtocolMessage create_telemetry_message(const domain::TelemetryReport& report) = 0;
    virtual ProtocolMessage create_ack_message(const std::string& command_id) = 0;
    virtual ProtocolMessage create_error_message(const std::string& error) = 0;
  };

  // JSON-based protocol handler
  class JsonProtocolHandler : public ProtocolHandler {
  public:
    std::string serialize_command(const domain::Command& command) override;
    std::string serialize_telemetry(const domain::TelemetryReport& report) override;
    std::string serialize_message(const ProtocolMessage& message) override;
    
    std::unique_ptr<domain::Command> deserialize_command(const std::string& data) override;
    std::unique_ptr<domain::TelemetryReport> deserialize_telemetry(const std::string& data) override;
    std::unique_ptr<ProtocolMessage> deserialize_message(const std::string& data) override;
    
    ProtocolMessage create_command_message(const domain::Command& command) override;
    ProtocolMessage create_telemetry_message(const domain::TelemetryReport& report) override;
    ProtocolMessage create_ack_message(const std::string& command_id) override;
    ProtocolMessage create_error_message(const std::string& error) override;
  };

} // namespace ccsim::net
