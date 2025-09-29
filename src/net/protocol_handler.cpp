#include "protocol_handler.h"

namespace ccsim::net {

  // JsonProtocolHandler implementation
  std::string JsonProtocolHandler::serialize_command(const domain::Command& command) {
    return command.to_json().dump();
  }

  std::string JsonProtocolHandler::serialize_telemetry(const domain::TelemetryReport& report) {
    return report.to_json().dump();
  }

  std::string JsonProtocolHandler::serialize_message(const ProtocolMessage& message) {
    return message.to_json().dump();
  }

  std::unique_ptr<domain::Command> JsonProtocolHandler::deserialize_command(const std::string& data) {
    try {
      auto j = nlohmann::json::parse(data);
      return domain::CommandFactory::create_from_json(j);
    } catch (const std::exception&) {
      return nullptr;
    }
  }

  std::unique_ptr<domain::TelemetryReport> JsonProtocolHandler::deserialize_telemetry(const std::string& data) {
    try {
      auto j = nlohmann::json::parse(data);
      return domain::TelemetryReport::from_json(j);
    } catch (const std::exception&) {
      return nullptr;
    }
  }

  std::unique_ptr<ProtocolMessage> JsonProtocolHandler::deserialize_message(const std::string& data) {
    try {
      auto j = nlohmann::json::parse(data);
      return ProtocolMessage::from_json(j);
    } catch (const std::exception&) {
      return nullptr;
    }
  }

  ProtocolMessage JsonProtocolHandler::create_command_message(const domain::Command& command) {
    ProtocolMessage msg;
    msg.type = MessageType::COMMAND;
    msg.payload = serialize_command(command);
    return msg;
  }

  ProtocolMessage JsonProtocolHandler::create_telemetry_message(const domain::TelemetryReport& report) {
    ProtocolMessage msg;
    msg.type = MessageType::TELEMETRY;
    msg.payload = serialize_telemetry(report);
    return msg;
  }

  ProtocolMessage JsonProtocolHandler::create_ack_message(const std::string& command_id) {
    ProtocolMessage msg;
    msg.type = MessageType::ACKNOWLEDGMENT;
    
    nlohmann::json payload;
    payload["command_id"] = command_id;
    payload["status"] = "success";
    msg.payload = payload.dump();
    
    return msg;
  }

  ProtocolMessage JsonProtocolHandler::create_error_message(const std::string& error) {
    ProtocolMessage msg;
    msg.type = MessageType::ERROR;
    
    nlohmann::json payload;
    payload["error"] = error;
    msg.payload = payload.dump();
    
    return msg;
  }

} // namespace ccsim::net
