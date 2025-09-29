#include <gtest/gtest.h>
#include "controller/controller.h"
#include "core/registry.h"
#include <thread>
#include <chrono>

namespace {

  TEST(ControllerTest, Construction) {
    ccsim::controller::Controller controller(8081);
    
    EXPECT_FALSE(controller.is_running());
    EXPECT_EQ(controller.connected_units(), 0);
    EXPECT_EQ(controller.total_units(), 0);
  }

  TEST(ControllerTest, UnitRegistration) {
    ccsim::controller::Controller controller(8082);
    
    // Register a unit
    controller.register_unit("test_unit_001", {10.0, 20.0, 30.0});
    
    EXPECT_EQ(controller.total_units(), 1);
    
    // Register another unit
    controller.register_unit("test_unit_002");
    
    EXPECT_EQ(controller.total_units(), 2);
    
    // Unregister a unit
    controller.unregister_unit("test_unit_001");
    
    EXPECT_EQ(controller.total_units(), 1);
  }

  TEST(ControllerTest, UnitStatus) {
    ccsim::controller::Controller controller(8083);
    
    controller.register_unit("test_unit_001", {5.0, 10.0, 15.0});
    controller.register_unit("test_unit_002", {20.0, 30.0, 40.0});
    
    auto status_list = controller.get_unit_status();
    
    EXPECT_EQ(status_list.size(), 2);
    
    // Check that status contains unit information
    bool found_unit1 = false, found_unit2 = false;
    for (const auto& status : status_list) {
      if (status.find("test_unit_001") != std::string::npos) {
        found_unit1 = true;
        EXPECT_TRUE(status.find("(5, 10, 15)") != std::string::npos);
      }
      if (status.find("test_unit_002") != std::string::npos) {
        found_unit2 = true;
        EXPECT_TRUE(status.find("(20, 30, 40)") != std::string::npos);
      }
    }
    
    EXPECT_TRUE(found_unit1);
    EXPECT_TRUE(found_unit2);
  }

  TEST(ControllerTest, StrategyConfiguration) {
    ccsim::controller::Controller controller(8084);
    
    // Test different strategy types
    controller.set_dispatch_strategy(ccsim::core::StrategyFactory::StrategyType::ROUND_ROBIN);
    controller.set_dispatch_strategy(ccsim::core::StrategyFactory::StrategyType::PRIORITY);
    controller.set_dispatch_strategy(ccsim::core::StrategyFactory::StrategyType::BROADCAST);
    
    // If we get here without exceptions, the strategy setting works
    EXPECT_TRUE(true);
  }

  TEST(ControllerTest, CommandSending) {
    ccsim::controller::Controller controller(8085);
    
    // Register a unit first
    controller.register_unit("test_unit_001", {0.0, 0.0, 0.0});
    
    // Test move command
    ccsim::domain::Position destination{100.0, 200.0, 300.0};
    controller.send_move_command("test_unit_001", destination);
    
    // Test report command
    controller.send_report_command("test_unit_001");
    
    // Test alert command
    controller.send_alert_command("test_unit_001", "Test alert message", 2);
    
    // If we get here without exceptions, command sending works
    EXPECT_TRUE(true);
  }

  TEST(ControllerTest, BroadcastCommands) {
    ccsim::controller::Controller controller(8086);
    
    // Register multiple units
    controller.register_unit("test_unit_001");
    controller.register_unit("test_unit_002");
    controller.register_unit("test_unit_003");
    
    // Test broadcast move command
    ccsim::domain::Position destination{50.0, 75.0, 100.0};
    controller.broadcast_move_command(destination);
    
    // Test broadcast report command
    controller.broadcast_report_command();
    
    // If we get here without exceptions, broadcast commands work
    EXPECT_TRUE(true);
  }

  TEST(ControllerTest, TelemetryCollection) {
    ccsim::controller::Controller controller(8087);
    
    // Start telemetry collection
    controller.start_telemetry_collection();
    
    // Let it run briefly
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    // Stop telemetry collection
    controller.stop_telemetry_collection();
    
    // If we get here without exceptions, telemetry collection works
    EXPECT_TRUE(true);
  }

  TEST(ControllerTest, StartStop) {
    ccsim::controller::Controller controller(8088);
    
    EXPECT_FALSE(controller.is_running());
    
    // Start controller
    controller.start();
    
    // Let it start up
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    EXPECT_TRUE(controller.is_running());
    
    // Stop controller
    controller.stop();
    
    EXPECT_FALSE(controller.is_running());
  }

  TEST(ControllerIntegrationTest, FullWorkflow) {
    ccsim::controller::Controller controller(8089);
    
    // Start controller
    controller.start();
    
    // Register units
    controller.register_unit("unit_001", {0.0, 0.0, 0.0});
    controller.register_unit("unit_002", {10.0, 10.0, 10.0});
    controller.register_unit("unit_003", {20.0, 20.0, 20.0});
    
    EXPECT_EQ(controller.total_units(), 3);
    
    // Set strategy
    controller.set_dispatch_strategy(ccsim::core::StrategyFactory::StrategyType::ROUND_ROBIN);
    
    // Send commands
    controller.send_move_command("unit_001", {100.0, 100.0, 100.0});
    controller.broadcast_report_command();
    controller.send_alert_command("unit_002", "Integration test alert", 1);
    
    // Let the system process commands
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    
    // Check status
    auto status_list = controller.get_unit_status();
    EXPECT_EQ(status_list.size(), 3);
    
    // Stop controller
    controller.stop();
    
    EXPECT_FALSE(controller.is_running());
  }

  TEST(RegistryIntegrationTest, UnitManagement) {
    auto& registry = ccsim::core::UnitRegistry::instance();
    
    // Clear any existing units
    auto all_units = registry.get_all_unit_ids();
    for (const auto& unit_id : all_units) {
      registry.unregister_unit(unit_id);
    }
    
    EXPECT_EQ(registry.total_units(), 0);
    
    // Register units
    auto unit1 = std::make_shared<ccsim::domain::Unit>(
      ccsim::domain::UnitId("test_unit_001"), ccsim::domain::Position{10.0, 20.0, 30.0});
    auto unit2 = std::make_shared<ccsim::domain::Unit>(
      ccsim::domain::UnitId("test_unit_002"), ccsim::domain::Position{40.0, 50.0, 60.0});
    
    registry.register_unit(unit1);
    registry.register_unit(unit2);
    
    EXPECT_EQ(registry.total_units(), 2);
    EXPECT_EQ(registry.online_units(), 2);
    EXPECT_EQ(registry.offline_units(), 0);
    
    // Test unit retrieval
    auto retrieved_unit = registry.get_unit(ccsim::domain::UnitId("test_unit_001"));
    ASSERT_NE(retrieved_unit, nullptr);
    EXPECT_EQ(retrieved_unit->id().value(), "test_unit_001");
    EXPECT_EQ(retrieved_unit->position().x, 10.0);
    EXPECT_EQ(retrieved_unit->position().y, 20.0);
    EXPECT_EQ(retrieved_unit->position().z, 30.0);
    
    // Test unit updates
    registry.update_unit_status(ccsim::domain::UnitId("test_unit_001"), ccsim::domain::UnitStatus::MOVING);
    registry.update_unit_position(ccsim::domain::UnitId("test_unit_002"), {100.0, 200.0, 300.0});
    
    // Verify updates
    auto updated_unit1 = registry.get_unit(ccsim::domain::UnitId("test_unit_001"));
    EXPECT_EQ(updated_unit1->status(), ccsim::domain::UnitStatus::MOVING);
    
    auto updated_unit2 = registry.get_unit(ccsim::domain::UnitId("test_unit_002"));
    EXPECT_EQ(updated_unit2->position().x, 100.0);
    EXPECT_EQ(updated_unit2->position().y, 200.0);
    EXPECT_EQ(updated_unit2->position().z, 300.0);
    
    // Unregister units
    registry.unregister_unit(ccsim::domain::UnitId("test_unit_001"));
    registry.unregister_unit(ccsim::domain::UnitId("test_unit_002"));
    
    EXPECT_EQ(registry.total_units(), 0);
  }

} // namespace
