#include <gtest/gtest.h>
#include "agent/agent.h"
#include "agent/command_executor.h"
#include <thread>
#include <chrono>

namespace {

  TEST(CommandExecutorTest, Construction) {
    ccsim::domain::UnitId unit_id("test_unit");
    ccsim::agent::CommandExecutor executor(unit_id);
    
    EXPECT_FALSE(executor.is_busy());
    EXPECT_FALSE(executor.is_operational());
    EXPECT_EQ(executor.get_battery_level(), 100.0);
    
    auto pos = executor.get_position();
    EXPECT_EQ(pos.x, 0.0);
    EXPECT_EQ(pos.y, 0.0);
    EXPECT_EQ(pos.z, 0.0);
  }

  TEST(CommandExecutorTest, OperationControl) {
    ccsim::domain::UnitId unit_id("test_unit");
    ccsim::agent::CommandExecutor executor(unit_id);
    
    EXPECT_FALSE(executor.is_operational());
    
    // Start operation
    executor.start_operation();
    EXPECT_TRUE(executor.is_operational());
    
    // Stop operation
    executor.stop_operation();
    EXPECT_FALSE(executor.is_operational());
  }

  TEST(CommandExecutorTest, PositionManagement) {
    ccsim::domain::UnitId unit_id("test_unit");
    ccsim::agent::CommandExecutor executor(unit_id);
    
    // Set initial position
    ccsim::domain::Position initial_pos{10.0, 20.0, 30.0};
    executor.set_position(initial_pos);
    
    auto pos = executor.get_position();
    EXPECT_EQ(pos.x, 10.0);
    EXPECT_EQ(pos.y, 20.0);
    EXPECT_EQ(pos.z, 30.0);
  }

  TEST(CommandExecutorTest, BatteryManagement) {
    ccsim::domain::UnitId unit_id("test_unit");
    ccsim::agent::CommandExecutor executor(unit_id);
    
    EXPECT_EQ(executor.get_battery_level(), 100.0);
    
    // Drain battery
    executor.set_battery_level(75.0);
    EXPECT_EQ(executor.get_battery_level(), 75.0);
    
    // Test clamping
    executor.set_battery_level(150.0); // Above 100
    EXPECT_EQ(executor.get_battery_level(), 100.0);
    
    executor.set_battery_level(-10.0); // Below 0
    EXPECT_EQ(executor.get_battery_level(), 0.0);
  }

  TEST(CommandExecutorTest, ExecutionDelay) {
    ccsim::domain::UnitId unit_id("test_unit");
    ccsim::agent::CommandExecutor executor(unit_id);
    
    executor.start_operation();
    
    // Set execution delay
    executor.set_execution_delay(std::chrono::milliseconds(50));
    
    // Create and execute a move command
    auto command = ccsim::domain::CommandFactory::create_move_command(
      unit_id, ccsim::domain::Position{100.0, 200.0, 300.0});
    
    auto start_time = std::chrono::steady_clock::now();
    
    executor.execute_command(std::move(command));
    
    // Wait for execution to complete
    while (executor.is_busy()) {
      std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    
    auto end_time = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
    
    // Should take at least the execution delay
    EXPECT_GE(duration.count(), 50);
    
    // Position should be updated
    auto pos = executor.get_position();
    EXPECT_EQ(pos.x, 100.0);
    EXPECT_EQ(pos.y, 200.0);
    EXPECT_EQ(pos.z, 300.0);
  }

  TEST(CommandExecutorTest, TelemetryGeneration) {
    ccsim::domain::UnitId unit_id("test_unit");
    ccsim::agent::CommandExecutor executor(unit_id);
    
    executor.start_operation();
    executor.set_position({25.0, 35.0, 45.0});
    
    auto telemetry = executor.generate_telemetry();
    
    EXPECT_EQ(telemetry.unit_id, unit_id);
    EXPECT_EQ(telemetry.position.x, 25.0);
    EXPECT_EQ(telemetry.position.y, 35.0);
    EXPECT_EQ(telemetry.position.z, 45.0);
    EXPECT_EQ(telemetry.status, ccsim::domain::UnitStatus::IDLE);
    EXPECT_GE(telemetry.battery_level, 0.0);
    EXPECT_LE(telemetry.battery_level, 100.0);
    EXPECT_GE(telemetry.cpu_usage, 0.0);
    EXPECT_LE(telemetry.cpu_usage, 100.0);
    EXPECT_GE(telemetry.memory_usage, 0.0);
    EXPECT_LE(telemetry.memory_usage, 100.0);
  }

  TEST(CommandExecutorTest, MoveCommandExecution) {
    ccsim::domain::UnitId unit_id("test_unit");
    ccsim::agent::CommandExecutor executor(unit_id);
    
    executor.start_operation();
    executor.set_execution_delay(std::chrono::milliseconds(10));
    
    // Create move command
    auto command = ccsim::domain::CommandFactory::create_move_command(
      unit_id, ccsim::domain::Position{50.0, 60.0, 70.0});
    
    executor.execute_command(std::move(command));
    
    // Wait for execution
    while (executor.is_busy()) {
      std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    
    // Verify position was updated
    auto pos = executor.get_position();
    EXPECT_EQ(pos.x, 50.0);
    EXPECT_EQ(pos.y, 60.0);
    EXPECT_EQ(pos.z, 70.0);
    
    // Battery should be drained
    EXPECT_LT(executor.get_battery_level(), 100.0);
  }

  TEST(CommandExecutorTest, ReportCommandExecution) {
    ccsim::domain::UnitId unit_id("test_unit");
    ccsim::agent::CommandExecutor executor(unit_id);
    
    executor.start_operation();
    
    // Create report command
    auto command = ccsim::domain::CommandFactory::create_report_command(unit_id);
    
    executor.execute_command(std::move(command));
    
    // Wait for execution
    while (executor.is_busy()) {
      std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    
    // Battery should be drained slightly
    EXPECT_LT(executor.get_battery_level(), 100.0);
  }

  TEST(CommandExecutorTest, AlertCommandExecution) {
    ccsim::domain::UnitId unit_id("test_unit");
    ccsim::agent::CommandExecutor executor(unit_id);
    
    executor.start_operation();
    
    // Create alert command
    auto command = ccsim::domain::CommandFactory::create_alert_command(
      unit_id, "Test alert message", 2);
    
    executor.execute_command(std::move(command));
    
    // Wait for execution
    while (executor.is_busy()) {
      std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    
    // Battery should be drained
    EXPECT_LT(executor.get_battery_level(), 100.0);
  }

  TEST(AgentTest, Construction) {
    ccsim::agent::Agent agent("localhost", 8080, "test_agent");
    
    EXPECT_EQ(agent.get_agent_id(), "test_agent");
    EXPECT_FALSE(agent.is_connected());
    EXPECT_FALSE(agent.is_running());
    
    auto pos = agent.get_position();
    EXPECT_EQ(pos.x, 0.0);
    EXPECT_EQ(pos.y, 0.0);
    EXPECT_EQ(pos.z, 0.0);
  }

  TEST(AgentTest, AutoGeneratedId) {
    ccsim::agent::Agent agent1("localhost", 8080);
    ccsim::agent::Agent agent2("localhost", 8080);
    
    // Both agents should have different auto-generated IDs
    EXPECT_NE(agent1.get_agent_id(), agent2.get_agent_id());
    EXPECT_TRUE(agent1.get_agent_id().find("agent_") == 0);
    EXPECT_TRUE(agent2.get_agent_id().find("agent_") == 0);
  }

  TEST(AgentTest, InitialPosition) {
    ccsim::agent::Agent agent("localhost", 8080, "test_agent");
    
    ccsim::domain::Position initial_pos{15.0, 25.0, 35.0};
    agent.set_initial_position(initial_pos);
    
    auto pos = agent.get_position();
    EXPECT_EQ(pos.x, 15.0);
    EXPECT_EQ(pos.y, 25.0);
    EXPECT_EQ(pos.z, 35.0);
  }

  TEST(AgentTest, TelemetryInterval) {
    ccsim::agent::Agent agent("localhost", 8080, "test_agent");
    
    // Set custom telemetry interval
    agent.set_telemetry_interval(std::chrono::seconds(10));
    
    // Start and stop telemetry reporting
    agent.start_telemetry_reporting();
    
    // Let it run briefly
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    agent.stop_telemetry_reporting();
    
    // If we get here without exceptions, telemetry reporting works
    EXPECT_TRUE(true);
  }

  TEST(AgentTest, StartStop) {
    ccsim::agent::Agent agent("localhost", 8080, "test_agent");
    
    EXPECT_FALSE(agent.is_running());
    
    // Start agent
    agent.start();
    
    EXPECT_TRUE(agent.is_running());
    
    // Stop agent
    agent.stop();
    
    EXPECT_FALSE(agent.is_running());
  }

  TEST(AgentIntegrationTest, FullWorkflow) {
    ccsim::agent::Agent agent("localhost", 8080, "integration_test_agent");
    
    // Set initial position
    agent.set_initial_position({100.0, 200.0, 300.0});
    
    // Start agent
    agent.start();
    
    // Set telemetry interval
    agent.set_telemetry_interval(std::chrono::seconds(1));
    
    // Let agent run briefly
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    
    // Verify position
    auto pos = agent.get_position();
    EXPECT_EQ(pos.x, 100.0);
    EXPECT_EQ(pos.y, 200.0);
    EXPECT_EQ(pos.z, 300.0);
    
    // Stop agent
    agent.stop();
    
    EXPECT_FALSE(agent.is_running());
  }

  TEST(AgentStressTest, MultipleAgents) {
    const int num_agents = 5;
    std::vector<std::unique_ptr<ccsim::agent::Agent>> agents;
    
    // Create multiple agents
    for (int i = 0; i < num_agents; ++i) {
      std::string agent_id = "stress_test_agent_" + std::to_string(i);
      auto agent = std::make_unique<ccsim::agent::Agent>("localhost", 8080, agent_id);
      
      // Set different initial positions
      ccsim::domain::Position pos{static_cast<double>(i * 10), 
                                  static_cast<double>(i * 20), 
                                  static_cast<double>(i * 30)};
      agent->set_initial_position(pos);
      
      agents.push_back(std::move(agent));
    }
    
    // Start all agents
    for (auto& agent : agents) {
      agent->start();
    }
    
    // Let them run briefly
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    
    // Verify all agents are running
    for (auto& agent : agents) {
      EXPECT_TRUE(agent->is_running());
    }
    
    // Stop all agents
    for (auto& agent : agents) {
      agent->stop();
    }
    
    // Verify all agents are stopped
    for (auto& agent : agents) {
      EXPECT_FALSE(agent->is_running());
    }
  }

} // namespace
