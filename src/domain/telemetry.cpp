#include "telemetry.h"

namespace ccsim::domain {

  // TelemetryData implementation
  TelemetryData::TelemetryData(const UnitId& id, const Position& pos, UnitStatus stat)
    : unit_id(id), position(pos), status(stat),
      timestamp(std::chrono::steady_clock::now()) {}

  nlohmann::json TelemetryData::to_json() const {
    nlohmann::json j;
    j["unit_id"] = unit_id.value();
    j["position"] = {{"x", position.x}, {"y", position.y}, {"z", position.z}};
    j["status"] = static_cast<int>(status);
    j["battery_level"] = battery_level;
    j["cpu_usage"] = cpu_usage;
    j["memory_usage"] = memory_usage;
    j["last_command_id"] = last_command_id;
    j["timestamp"] = std::chrono::duration_cast<std::chrono::milliseconds>(
      timestamp.time_since_epoch()).count();
    return j;
  }

  TelemetryData TelemetryData::from_json(const nlohmann::json& j) {
    auto unit_id = UnitId(j["unit_id"]);
    auto pos = j["position"];
    Position position{pos["x"], pos["y"], pos["z"]};
    auto status = static_cast<UnitStatus>(j["status"]);
    
    TelemetryData data(unit_id, position, status);
    data.battery_level = j.value("battery_level", 100.0);
    data.cpu_usage = j.value("cpu_usage", 0.0);
    data.memory_usage = j.value("memory_usage", 0.0);
    data.last_command_id = j.value("last_command_id", "");
    
    if (j.contains("timestamp")) {
      data.timestamp = std::chrono::steady_clock::time_point(
        std::chrono::milliseconds(j["timestamp"]));
    }
    
    return data;
  }

  // TelemetryReport implementation
  TelemetryReport::TelemetryReport(const TelemetryData& data) : data_(data) {}

  nlohmann::json TelemetryReport::to_json() const {
    nlohmann::json j;
    j["type"] = "telemetry";
    j["data"] = data_.to_json();
    return j;
  }

  std::unique_ptr<TelemetryReport> TelemetryReport::from_json(const nlohmann::json& j) {
    if (j.value("type", "") != "telemetry" || !j.contains("data")) {
      return nullptr;
    }
    
    auto data = TelemetryData::from_json(j["data"]);
    return std::make_unique<TelemetryReport>(data);
  }

} // namespace ccsim::domain
