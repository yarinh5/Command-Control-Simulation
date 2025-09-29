#include <gtest/gtest.h>
#include "domain/unit.h"
#include "domain/command.h"
#include "domain/telemetry.h"
#include <nlohmann/json.hpp>

namespace {

  TEST(UnitIdTest, Construction) {
    ccsim::domain::UnitId unit_id("test_unit_001");
    EXPECT_EQ(unit_id.value(), "test_unit_001");
  }

  TEST(UnitIdTest, Equality) {
    ccsim::domain::UnitId id1("unit_001");
    ccsim::domain::UnitId id2("unit_001");
    ccsim::domain::UnitId id3("unit_002");
    
    EXPECT_EQ(id1, id2);
    EXPECT_NE(id1, id3);
  }

  TEST(UnitIdTest, Hash) {
    ccsim::domain::UnitId id1("unit_001");
    ccsim::domain::UnitId id2("unit_001");
    ccsim::domain::UnitId id3("unit_002");
    
    ccsim::domain::UnitId::Hash hasher;
    EXPECT_EQ(hasher(id1), hasher(id2));
    EXPECT_NE(hasher(id1), hasher(id3));
  }

  TEST(UnitTest, Construction) {
    ccsim::domain::UnitId unit_id("test_unit");
    ccsim::domain::Position initial_pos{10.0, 20.0, 30.0};
    ccsim::domain::Unit unit(unit_id, initial_pos);
    
    EXPECT_EQ(unit.id(), unit_id);
    EXPECT_EQ(unit.position().x, 10.0);
    EXPECT_EQ(unit.position().y, 20.0);
    EXPECT_EQ(unit.position().z, 30.0);
    EXPECT_EQ(unit.status(), ccsim::domain::UnitStatus::IDLE);
  }

  TEST(UnitTest, PositionUpdate) {
    ccsim::domain::UnitId unit_id("test_unit");
    ccsim::domain::Unit unit(unit_id);
    
    ccsim::domain::Position new_pos{5.0, 15.0, 25.0};
    unit.set_position(new_pos);
    
    EXPECT_EQ(unit.position().x, 5.0);
    EXPECT_EQ(unit.position().y, 15.0);
    EXPECT_EQ(unit.position().z, 25.0);
  }

  TEST(UnitTest, StatusUpdate) {
    ccsim::domain::UnitId unit_id("test_unit");
    ccsim::domain::Unit unit(unit_id);
    
    unit.set_status(ccsim::domain::UnitStatus::MOVING);
    EXPECT_EQ(unit.status(), ccsim::domain::UnitStatus::MOVING);
    
    unit.set_status(ccsim::domain::UnitStatus::BUSY);
    EXPECT_EQ(unit.status(), ccsim::domain::UnitStatus::BUSY);
  }

  TEST(UnitTest, OnlineStatus) {
    ccsim::domain::UnitId unit_id("test_unit");
    ccsim::domain::Unit unit(unit_id);
    
    // Should be online initially (just created)
    EXPECT_TRUE(unit.is_online());
  }

  TEST(CommandTest, Construction) {
    ccsim::domain::UnitId target_id("target_unit");
    ccsim::domain::Command command(target_id, ccsim::domain::CommandType::MOVE);
    
    EXPECT_EQ(command.target_id(), target_id);
    EXPECT_EQ(command.type(), ccsim::domain::CommandType::MOVE);
    EXPECT_FALSE(command.id().empty());
  }

  TEST(CommandTest, JsonSerialization) {
    ccsim::domain::UnitId target_id("target_unit");
    ccsim::domain::Command command(target_id, ccsim::domain::CommandType::REPORT);
    
    auto json = command.to_json();
    
    EXPECT_TRUE(json.contains("id"));
    EXPECT_TRUE(json.contains("target_id"));
    EXPECT_TRUE(json.contains("type"));
    EXPECT_TRUE(json.contains("timestamp"));
    
    EXPECT_EQ(json["target_id"], "target_unit");
    EXPECT_EQ(json["type"], static_cast<int>(ccsim::domain::CommandType::REPORT));
  }

  TEST(CommandTest, JsonDeserialization) {
    nlohmann::json j;
    j["target_id"] = "test_unit";
    j["type"] = static_cast<int>(ccsim::domain::CommandType::MOVE);
    j["payload"] = nlohmann::json::object();
    
    auto command = ccsim::domain::Command::from_json(j);
    ASSERT_NE(command, nullptr);
    
    EXPECT_EQ(command->target_id().value(), "test_unit");
    EXPECT_EQ(command->type(), ccsim::domain::CommandType::MOVE);
  }

  TEST(MoveCommandTest, Construction) {
    ccsim::domain::UnitId target_id("target_unit");
    ccsim::domain::Position destination{100.0, 200.0, 300.0};
    
    ccsim::domain::MoveCommand move_cmd(target_id, destination);
    
    EXPECT_EQ(move_cmd.type(), ccsim::domain::CommandType::MOVE);
    EXPECT_EQ(move_cmd.target_id(), target_id);
    EXPECT_EQ(move_cmd.destination().x, 100.0);
    EXPECT_EQ(move_cmd.destination().y, 200.0);
    EXPECT_EQ(move_cmd.destination().z, 300.0);
  }

  TEST(AlertCommandTest, Construction) {
    ccsim::domain::UnitId target_id("target_unit");
    std::string message = "Test alert message";
    int severity = 2;
    
    ccsim::domain::AlertCommand alert_cmd(target_id, message, severity);
    
    EXPECT_EQ(alert_cmd.type(), ccsim::domain::CommandType::ALERT);
    EXPECT_EQ(alert_cmd.target_id(), target_id);
    EXPECT_EQ(alert_cmd.message(), message);
    EXPECT_EQ(alert_cmd.severity(), severity);
  }

  TEST(CommandFactoryTest, MoveCommand) {
    ccsim::domain::UnitId target_id("target_unit");
    ccsim::domain::Position destination{50.0, 75.0, 100.0};
    
    auto command = ccsim::domain::CommandFactory::create_move_command(target_id, destination);
    ASSERT_NE(command, nullptr);
    
    EXPECT_EQ(command->type(), ccsim::domain::CommandType::MOVE);
    EXPECT_EQ(command->target_id(), target_id);
  }

  TEST(CommandFactoryTest, ReportCommand) {
    ccsim::domain::UnitId target_id("target_unit");
    
    auto command = ccsim::domain::CommandFactory::create_report_command(target_id);
    ASSERT_NE(command, nullptr);
    
    EXPECT_EQ(command->type(), ccsim::domain::CommandType::REPORT);
    EXPECT_EQ(command->target_id(), target_id);
  }

  TEST(CommandFactoryTest, AlertCommand) {
    ccsim::domain::UnitId target_id("target_unit");
    std::string message = "Factory test alert";
    
    auto command = ccsim::domain::CommandFactory::create_alert_command(target_id, message);
    ASSERT_NE(command, nullptr);
    
    EXPECT_EQ(command->type(), ccsim::domain::CommandType::ALERT);
    EXPECT_EQ(command->target_id(), target_id);
  }

  TEST(TelemetryDataTest, Construction) {
    ccsim::domain::UnitId unit_id("test_unit");
    ccsim::domain::Position position{1.0, 2.0, 3.0};
    ccsim::domain::UnitStatus status = ccsim::domain::UnitStatus::IDLE;
    
    ccsim::domain::TelemetryData data(unit_id, position, status);
    
    EXPECT_EQ(data.unit_id, unit_id);
    EXPECT_EQ(data.position.x, 1.0);
    EXPECT_EQ(data.position.y, 2.0);
    EXPECT_EQ(data.position.z, 3.0);
    EXPECT_EQ(data.status, status);
    EXPECT_EQ(data.battery_level, 100.0);
    EXPECT_EQ(data.cpu_usage, 0.0);
    EXPECT_EQ(data.memory_usage, 0.0);
  }

  TEST(TelemetryDataTest, JsonSerialization) {
    ccsim::domain::UnitId unit_id("test_unit");
    ccsim::domain::Position position{10.0, 20.0, 30.0};
    ccsim::domain::UnitStatus status = ccsim::domain::UnitStatus::MOVING;
    
    ccsim::domain::TelemetryData data(unit_id, position, status);
    data.battery_level = 75.5;
    data.cpu_usage = 45.2;
    data.memory_usage = 60.8;
    
    auto json = data.to_json();
    
    EXPECT_TRUE(json.contains("unit_id"));
    EXPECT_TRUE(json.contains("position"));
    EXPECT_TRUE(json.contains("status"));
    EXPECT_TRUE(json.contains("battery_level"));
    EXPECT_TRUE(json.contains("cpu_usage"));
    EXPECT_TRUE(json.contains("memory_usage"));
    EXPECT_TRUE(json.contains("timestamp"));
    
    EXPECT_EQ(json["unit_id"], "test_unit");
    EXPECT_EQ(json["battery_level"], 75.5);
    EXPECT_EQ(json["cpu_usage"], 45.2);
    EXPECT_EQ(json["memory_usage"], 60.8);
  }

  TEST(TelemetryDataTest, JsonDeserialization) {
    nlohmann::json j;
    j["unit_id"] = "test_unit";
    j["position"] = {{"x", 15.0}, {"y", 25.0}, {"z", 35.0}};
    j["status"] = static_cast<int>(ccsim::domain::UnitStatus::BUSY);
    j["battery_level"] = 80.0;
    j["cpu_usage"] = 30.0;
    j["memory_usage"] = 50.0;
    j["timestamp"] = 1234567890;
    
    auto data = ccsim::domain::TelemetryData::from_json(j);
    
    EXPECT_EQ(data.unit_id.value(), "test_unit");
    EXPECT_EQ(data.position.x, 15.0);
    EXPECT_EQ(data.position.y, 25.0);
    EXPECT_EQ(data.position.z, 35.0);
    EXPECT_EQ(data.status, ccsim::domain::UnitStatus::BUSY);
    EXPECT_EQ(data.battery_level, 80.0);
    EXPECT_EQ(data.cpu_usage, 30.0);
    EXPECT_EQ(data.memory_usage, 50.0);
  }

  TEST(TelemetryReportTest, Construction) {
    ccsim::domain::UnitId unit_id("test_unit");
    ccsim::domain::Position position{5.0, 10.0, 15.0};
    ccsim::domain::UnitStatus status = ccsim::domain::UnitStatus::IDLE;
    
    ccsim::domain::TelemetryData data(unit_id, position, status);
    ccsim::domain::TelemetryReport report(data);
    
    EXPECT_EQ(report.data().unit_id, unit_id);
    EXPECT_EQ(report.data().position.x, 5.0);
    EXPECT_EQ(report.data().position.y, 10.0);
    EXPECT_EQ(report.data().position.z, 15.0);
    EXPECT_EQ(report.data().status, status);
  }

  TEST(TelemetryReportTest, JsonSerialization) {
    ccsim::domain::UnitId unit_id("test_unit");
    ccsim::domain::Position position{1.0, 2.0, 3.0};
    ccsim::domain::UnitStatus status = ccsim::domain::UnitStatus::MOVING;
    
    ccsim::domain::TelemetryData data(unit_id, position, status);
    ccsim::domain::TelemetryReport report(data);
    
    auto json = report.to_json();
    
    EXPECT_TRUE(json.contains("type"));
    EXPECT_TRUE(json.contains("data"));
    EXPECT_EQ(json["type"], "telemetry");
    EXPECT_EQ(json["data"]["unit_id"], "test_unit");
  }

} // namespace
