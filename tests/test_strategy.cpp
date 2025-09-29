#include <gtest/gtest.h>
#include "core/strategy.h"
#include "domain/command.h"
#include "domain/unit.h"

namespace {

  TEST(RoundRobinStrategyTest, SelectTargetsForMove) {
    ccsim::core::RoundRobinStrategy strategy;
    
    std::vector<ccsim::domain::UnitId> available_units = {
      ccsim::domain::UnitId("unit_001"),
      ccsim::domain::UnitId("unit_002"),
      ccsim::domain::UnitId("unit_003")
    };
    
    ccsim::domain::UnitId target_id("unit_001");
    auto command = ccsim::domain::CommandFactory::create_move_command(target_id, {10.0, 20.0, 30.0});
    
    // First call should select first unit
    auto targets1 = strategy.select_targets(available_units, *command);
    EXPECT_EQ(targets1.size(), 1);
    EXPECT_EQ(targets1[0], ccsim::domain::UnitId("unit_001"));
    
    // Second call should select second unit
    auto targets2 = strategy.select_targets(available_units, *command);
    EXPECT_EQ(targets2.size(), 1);
    EXPECT_EQ(targets2[0], ccsim::domain::UnitId("unit_002"));
    
    // Third call should select third unit
    auto targets3 = strategy.select_targets(available_units, *command);
    EXPECT_EQ(targets3.size(), 1);
    EXPECT_EQ(targets3[0], ccsim::domain::UnitId("unit_003"));
    
    // Fourth call should wrap around to first unit
    auto targets4 = strategy.select_targets(available_units, *command);
    EXPECT_EQ(targets4.size(), 1);
    EXPECT_EQ(targets4[0], ccsim::domain::UnitId("unit_001"));
  }

  TEST(RoundRobinStrategyTest, SelectTargetsForReport) {
    ccsim::core::RoundRobinStrategy strategy;
    
    std::vector<ccsim::domain::UnitId> available_units = {
      ccsim::domain::UnitId("unit_001"),
      ccsim::domain::UnitId("unit_002"),
      ccsim::domain::UnitId("unit_003")
    };
    
    ccsim::domain::UnitId target_id("unit_001");
    auto command = ccsim::domain::CommandFactory::create_report_command(target_id);
    
    // REPORT commands should select all units
    auto targets = strategy.select_targets(available_units, *command);
    EXPECT_EQ(targets.size(), 3);
    EXPECT_EQ(targets[0], ccsim::domain::UnitId("unit_001"));
    EXPECT_EQ(targets[1], ccsim::domain::UnitId("unit_002"));
    EXPECT_EQ(targets[2], ccsim::domain::UnitId("unit_003"));
  }

  TEST(RoundRobinStrategyTest, EmptyUnitsList) {
    ccsim::core::RoundRobinStrategy strategy;
    
    std::vector<ccsim::domain::UnitId> available_units;
    
    ccsim::domain::UnitId target_id("unit_001");
    auto command = ccsim::domain::CommandFactory::create_move_command(target_id, {10.0, 20.0, 30.0});
    
    auto targets = strategy.select_targets(available_units, *command);
    EXPECT_TRUE(targets.empty());
  }

  TEST(PriorityStrategyTest, SelectTargetsByPriority) {
    ccsim::core::PriorityStrategy strategy;
    
    ccsim::domain::UnitId unit1("unit_001");
    ccsim::domain::UnitId unit2("unit_002");
    ccsim::domain::UnitId unit3("unit_003");
    
    // Set priorities
    strategy.set_unit_priority(unit1, 10); // Highest priority
    strategy.set_unit_priority(unit2, 5);  // Medium priority
    strategy.set_unit_priority(unit3, 1);  // Lowest priority
    
    std::vector<ccsim::domain::UnitId> available_units = {unit2, unit1, unit3};
    
    ccsim::domain::UnitId target_id("unit_001");
    auto command = ccsim::domain::CommandFactory::create_move_command(target_id, {10.0, 20.0, 30.0});
    
    // Should select highest priority unit
    auto targets = strategy.select_targets(available_units, *command);
    EXPECT_EQ(targets.size(), 1);
    EXPECT_EQ(targets[0], unit1); // unit_001 has highest priority
  }

  TEST(PriorityStrategyTest, SelectTargetsForReport) {
    ccsim::core::PriorityStrategy strategy;
    
    ccsim::domain::UnitId unit1("unit_001");
    ccsim::domain::UnitId unit2("unit_002");
    ccsim::domain::UnitId unit3("unit_003");
    ccsim::domain::UnitId unit4("unit_004");
    
    // Set priorities
    strategy.set_unit_priority(unit1, 10);
    strategy.set_unit_priority(unit2, 5);
    strategy.set_unit_priority(unit3, 1);
    strategy.set_unit_priority(unit4, 8);
    
    std::vector<ccsim::domain::UnitId> available_units = {unit2, unit1, unit3, unit4};
    
    ccsim::domain::UnitId target_id("unit_001");
    auto command = ccsim::domain::CommandFactory::create_report_command(target_id);
    
    // Should select top 3 priority units
    auto targets = strategy.select_targets(available_units, *command);
    EXPECT_EQ(targets.size(), 3);
    EXPECT_EQ(targets[0], unit1); // Priority 10
    EXPECT_EQ(targets[1], unit4); // Priority 8
    EXPECT_EQ(targets[2], unit2); // Priority 5
  }

  TEST(PriorityStrategyTest, DefaultPriority) {
    ccsim::core::PriorityStrategy strategy;
    
    ccsim::domain::UnitId unit1("unit_001");
    ccsim::domain::UnitId unit2("unit_002");
    
    // Set priority for only one unit
    strategy.set_unit_priority(unit1, 10);
    
    std::vector<ccsim::domain::UnitId> available_units = {unit1, unit2};
    
    ccsim::domain::UnitId target_id("unit_001");
    auto command = ccsim::domain::CommandFactory::create_move_command(target_id, {10.0, 20.0, 30.0});
    
    // Should select unit with explicit priority
    auto targets = strategy.select_targets(available_units, *command);
    EXPECT_EQ(targets.size(), 1);
    EXPECT_EQ(targets[0], unit1);
  }

  TEST(BroadcastStrategyTest, SelectAllTargets) {
    ccsim::core::BroadcastStrategy strategy;
    
    std::vector<ccsim::domain::UnitId> available_units = {
      ccsim::domain::UnitId("unit_001"),
      ccsim::domain::UnitId("unit_002"),
      ccsim::domain::UnitId("unit_003")
    };
    
    ccsim::domain::UnitId target_id("unit_001");
    auto command = ccsim::domain::CommandFactory::create_move_command(target_id, {10.0, 20.0, 30.0});
    
    // Should select all available units
    auto targets = strategy.select_targets(available_units, *command);
    EXPECT_EQ(targets.size(), 3);
    EXPECT_EQ(targets[0], ccsim::domain::UnitId("unit_001"));
    EXPECT_EQ(targets[1], ccsim::domain::UnitId("unit_002"));
    EXPECT_EQ(targets[2], ccsim::domain::UnitId("unit_003"));
  }

  TEST(BroadcastStrategyTest, EmptyUnitsList) {
    ccsim::core::BroadcastStrategy strategy;
    
    std::vector<ccsim::domain::UnitId> available_units;
    
    ccsim::domain::UnitId target_id("unit_001");
    auto command = ccsim::domain::CommandFactory::create_move_command(target_id, {10.0, 20.0, 30.0});
    
    auto targets = strategy.select_targets(available_units, *command);
    EXPECT_TRUE(targets.empty());
  }

  TEST(StrategyFactoryTest, CreateRoundRobinStrategy) {
    auto strategy = ccsim::core::StrategyFactory::create_strategy(
      ccsim::core::StrategyFactory::StrategyType::ROUND_ROBIN);
    
    ASSERT_NE(strategy, nullptr);
    
    // Test that it's actually a RoundRobinStrategy by using it
    std::vector<ccsim::domain::UnitId> available_units = {
      ccsim::domain::UnitId("unit_001"),
      ccsim::domain::UnitId("unit_002")
    };
    
    ccsim::domain::UnitId target_id("unit_001");
    auto command = ccsim::domain::CommandFactory::create_move_command(target_id, {10.0, 20.0, 30.0});
    
    auto targets1 = strategy->select_targets(available_units, *command);
    auto targets2 = strategy->select_targets(available_units, *command);
    
    EXPECT_EQ(targets1[0], ccsim::domain::UnitId("unit_001"));
    EXPECT_EQ(targets2[0], ccsim::domain::UnitId("unit_002"));
  }

  TEST(StrategyFactoryTest, CreatePriorityStrategy) {
    auto strategy = ccsim::core::StrategyFactory::create_strategy(
      ccsim::core::StrategyFactory::StrategyType::PRIORITY);
    
    ASSERT_NE(strategy, nullptr);
    
    // Cast to PriorityStrategy to test priority functionality
    auto priority_strategy = dynamic_cast<ccsim::core::PriorityStrategy*>(strategy.get());
    ASSERT_NE(priority_strategy, nullptr);
    
    ccsim::domain::UnitId unit1("unit_001");
    ccsim::domain::UnitId unit2("unit_002");
    
    priority_strategy->set_unit_priority(unit1, 10);
    priority_strategy->set_unit_priority(unit2, 5);
    
    std::vector<ccsim::domain::UnitId> available_units = {unit1, unit2};
    
    ccsim::domain::UnitId target_id("unit_001");
    auto command = ccsim::domain::CommandFactory::create_move_command(target_id, {10.0, 20.0, 30.0});
    
    auto targets = strategy->select_targets(available_units, *command);
    EXPECT_EQ(targets[0], unit1); // Higher priority
  }

  TEST(StrategyFactoryTest, CreateBroadcastStrategy) {
    auto strategy = ccsim::core::StrategyFactory::create_strategy(
      ccsim::core::StrategyFactory::StrategyType::BROADCAST);
    
    ASSERT_NE(strategy, nullptr);
    
    std::vector<ccsim::domain::UnitId> available_units = {
      ccsim::domain::UnitId("unit_001"),
      ccsim::domain::UnitId("unit_002"),
      ccsim::domain::UnitId("unit_003")
    };
    
    ccsim::domain::UnitId target_id("unit_001");
    auto command = ccsim::domain::CommandFactory::create_move_command(target_id, {10.0, 20.0, 30.0});
    
    auto targets = strategy->select_targets(available_units, *command);
    EXPECT_EQ(targets.size(), 3); // All units selected
  }

  TEST(CommandQueueTest, EnqueueDequeue) {
    ccsim::core::CommandQueue queue;
    
    EXPECT_TRUE(queue.empty());
    EXPECT_EQ(queue.size(), 0);
    
    // Enqueue a command
    auto command1 = ccsim::domain::CommandFactory::create_move_command(
      ccsim::domain::UnitId("unit_001"), {10.0, 20.0, 30.0});
    
    queue.enqueue(std::move(command1));
    
    EXPECT_FALSE(queue.empty());
    EXPECT_EQ(queue.size(), 1);
    
    // Enqueue another command with higher priority
    auto command2 = ccsim::domain::CommandFactory::create_report_command(
      ccsim::domain::UnitId("unit_002"));
    
    queue.enqueue(std::move(command2), 10); // Higher priority
    
    EXPECT_EQ(queue.size(), 2);
    
    // Dequeue should return higher priority command first
    auto dequeued = queue.dequeue();
    ASSERT_NE(dequeued, nullptr);
    EXPECT_EQ(dequeued->type(), ccsim::domain::CommandType::REPORT);
    
    EXPECT_EQ(queue.size(), 1);
    
    // Dequeue the remaining command
    dequeued = queue.dequeue();
    ASSERT_NE(dequeued, nullptr);
    EXPECT_EQ(dequeued->type(), ccsim::domain::CommandType::MOVE);
    
    EXPECT_TRUE(queue.empty());
    EXPECT_EQ(queue.size(), 0);
    
    // Dequeue from empty queue
    dequeued = queue.dequeue();
    EXPECT_EQ(dequeued, nullptr);
  }

  TEST(CommandQueueTest, PriorityOrdering) {
    ccsim::core::CommandQueue queue;
    
    // Enqueue commands with different priorities
    auto command1 = ccsim::domain::CommandFactory::create_move_command(
      ccsim::domain::UnitId("unit_001"), {10.0, 20.0, 30.0});
    queue.enqueue(std::move(command1), 1); // Low priority
    
    auto command2 = ccsim::domain::CommandFactory::create_alert_command(
      ccsim::domain::UnitId("unit_002"), "Alert message");
    queue.enqueue(std::move(command2), 10); // High priority
    
    auto command3 = ccsim::domain::CommandFactory::create_report_command(
      ccsim::domain::UnitId("unit_003"));
    queue.enqueue(std::move(command3), 5); // Medium priority
    
    // Dequeue and verify priority order
    auto dequeued1 = queue.dequeue();
    EXPECT_EQ(dequeued1->type(), ccsim::domain::CommandType::ALERT); // Highest priority
    
    auto dequeued2 = queue.dequeue();
    EXPECT_EQ(dequeued2->type(), ccsim::domain::CommandType::REPORT); // Medium priority
    
    auto dequeued3 = queue.dequeue();
    EXPECT_EQ(dequeued3->type(), ccsim::domain::CommandType::MOVE); // Lowest priority
    
    EXPECT_TRUE(queue.empty());
  }

} // namespace
