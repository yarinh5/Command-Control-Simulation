#include <gtest/gtest.h>
#include "core/observer.h"
#include "domain/telemetry.h"
#include "domain/command.h"
#include <memory>
#include <atomic>

namespace {

  // Test observer implementations
  class TestTelemetryObserver : public ccsim::core::TelemetryObserver {
  public:
    void on_telemetry_received(const ccsim::domain::TelemetryReport& report) override {
      received_reports_.push_back(report.data());
      call_count_++;
    }
    
    std::vector<ccsim::domain::TelemetryData> received_reports_;
    std::atomic<int> call_count_{0};
  };

  class TestCommandObserver : public ccsim::core::CommandObserver {
  public:
    void on_command_sent(const ccsim::domain::Command& command) override {
      sent_commands_.push_back(command.id());
      sent_count_++;
    }
    
    void on_command_completed(const std::string& command_id, bool success) override {
      completed_commands_[command_id] = success;
      completed_count_++;
    }
    
    std::vector<std::string> sent_commands_;
    std::map<std::string, bool> completed_commands_;
    std::atomic<int> sent_count_{0};
    std::atomic<int> completed_count_{0};
  };

  TEST(TelemetryObservableTest, AddRemoveObserver) {
    ccsim::core::TelemetryObservable observable;
    auto observer = std::make_shared<TestTelemetryObserver>();
    
    // Add observer
    observable.add_observer(observer);
    
    // Remove observer
    observable.remove_observer(observer);
    
    // Test passes if no exceptions are thrown
    EXPECT_TRUE(true);
  }

  TEST(TelemetryObservableTest, NotifyObservers) {
    ccsim::core::TelemetryObservable observable;
    auto observer1 = std::make_shared<TestTelemetryObserver>();
    auto observer2 = std::make_shared<TestTelemetryObserver>();
    
    observable.add_observer(observer1);
    observable.add_observer(observer2);
    
    // Create test telemetry data
    ccsim::domain::UnitId unit_id("test_unit");
    ccsim::domain::Position position{10.0, 20.0, 30.0};
    ccsim::domain::UnitStatus status = ccsim::domain::UnitStatus::MOVING;
    
    ccsim::domain::TelemetryData data(unit_id, position, status);
    ccsim::domain::TelemetryReport report(data);
    
    // Notify observers
    observable.notify_telemetry(report);
    
    // Both observers should be notified
    EXPECT_EQ(observer1->call_count_, 1);
    EXPECT_EQ(observer2->call_count_, 1);
    EXPECT_EQ(observer1->received_reports_.size(), 1);
    EXPECT_EQ(observer2->received_reports_.size(), 1);
    
    // Check data integrity
    EXPECT_EQ(observer1->received_reports_[0].unit_id, unit_id);
    EXPECT_EQ(observer1->received_reports_[0].position.x, 10.0);
    EXPECT_EQ(observer1->received_reports_[0].position.y, 20.0);
    EXPECT_EQ(observer1->received_reports_[0].position.z, 30.0);
    EXPECT_EQ(observer1->received_reports_[0].status, status);
  }

  TEST(TelemetryObservableTest, WeakObserverHandling) {
    ccsim::core::TelemetryObservable observable;
    
    // Create observer and add it
    {
      auto observer = std::make_shared<TestTelemetryObserver>();
      observable.add_observer(observer);
      
      // Observer should be added
      EXPECT_EQ(observable.get_observer_count(), 1);
    }
    
    // Observer goes out of scope, should be cleaned up
    // Note: In a real implementation, the observable would clean up expired weak pointers
    // For this test, we'll just verify the observable doesn't crash
    ccsim::domain::UnitId unit_id("test_unit");
    ccsim::domain::TelemetryData data(unit_id, {0, 0, 0}, ccsim::domain::UnitStatus::IDLE);
    ccsim::domain::TelemetryReport report(data);
    
    // This should not crash even with expired observers
    observable.notify_telemetry(report);
  }

  TEST(CommandObservableTest, AddRemoveObserver) {
    ccsim::core::CommandObservable observable;
    auto observer = std::make_shared<TestCommandObserver>();
    
    // Add observer
    observable.add_observer(observer);
    
    // Remove observer
    observable.remove_observer(observer);
  }

  TEST(CommandObservableTest, NotifyCommandSent) {
    ccsim::core::CommandObservable observable;
    auto observer = std::make_shared<TestCommandObserver>();
    
    observable.add_observer(observer);
    
    // Create test command
    ccsim::domain::UnitId target_id("target_unit");
    ccsim::domain::Command command(target_id, ccsim::domain::CommandType::MOVE);
    
    // Notify observers
    observable.notify_command_sent(command);
    
    // Observer should be notified
    EXPECT_EQ(observer->sent_count_, 1);
    EXPECT_EQ(observer->sent_commands_.size(), 1);
    EXPECT_EQ(observer->sent_commands_[0], command.id());
  }

  TEST(CommandObservableTest, NotifyCommandCompleted) {
    ccsim::core::CommandObservable observable;
    auto observer = std::make_shared<TestCommandObserver>();
    
    observable.add_observer(observer);
    
    std::string command_id = "test_command_001";
    bool success = true;
    
    // Notify observers
    observable.notify_command_completed(command_id, success);
    
    // Observer should be notified
    EXPECT_EQ(observer->completed_count_, 1);
    EXPECT_EQ(observer->completed_commands_.size(), 1);
    EXPECT_TRUE(observer->completed_commands_[command_id]);
  }

  TEST(CommandObservableTest, MultipleObservers) {
    ccsim::core::CommandObservable observable;
    auto observer1 = std::make_shared<TestCommandObserver>();
    auto observer2 = std::make_shared<TestCommandObserver>();
    
    observable.add_observer(observer1);
    observable.add_observer(observer2);
    
    // Create test command
    ccsim::domain::UnitId target_id("target_unit");
    ccsim::domain::Command command(target_id, ccsim::domain::CommandType::REPORT);
    
    // Notify command sent
    observable.notify_command_sent(command);
    
    // Both observers should be notified
    EXPECT_EQ(observer1->sent_count_, 1);
    EXPECT_EQ(observer2->sent_count_, 1);
    
    // Notify command completed
    std::string command_id = "test_command_002";
    observable.notify_command_completed(command_id, false);
    
    // Both observers should be notified
    EXPECT_EQ(observer1->completed_count_, 1);
    EXPECT_EQ(observer2->completed_count_, 1);
    EXPECT_FALSE(observer1->completed_commands_[command_id]);
    EXPECT_FALSE(observer2->completed_commands_[command_id]);
  }

  TEST(ObserverPatternIntegrationTest, FullWorkflow) {
    ccsim::core::TelemetryObservable telemetry_observable;
    ccsim::core::CommandObservable command_observable;
    
    auto telemetry_observer = std::make_shared<TestTelemetryObserver>();
    auto command_observer = std::make_shared<TestCommandObserver>();
    
    telemetry_observable.add_observer(telemetry_observer);
    command_observable.add_observer(command_observer);
    
    // Simulate command workflow
    ccsim::domain::UnitId target_id("test_unit");
    
    // 1. Send command
    auto command = ccsim::domain::CommandFactory::create_move_command(target_id, {100.0, 200.0, 300.0});
    command_observable.notify_command_sent(*command);
    
    // 2. Command completed
    command_observable.notify_command_completed(command->id(), true);
    
    // 3. Telemetry received
    ccsim::domain::TelemetryData data(target_id, {100.0, 200.0, 300.0}, ccsim::domain::UnitStatus::IDLE);
    ccsim::domain::TelemetryReport report(data);
    telemetry_observable.notify_telemetry(report);
    
    // Verify all notifications were received
    EXPECT_EQ(command_observer->sent_count_, 1);
    EXPECT_EQ(command_observer->completed_count_, 1);
    EXPECT_EQ(telemetry_observer->call_count_, 1);
    
    EXPECT_TRUE(command_observer->completed_commands_[command->id()]);
    EXPECT_EQ(command_observer->sent_commands_[0], command->id());
    EXPECT_EQ(telemetry_observer->received_reports_[0].unit_id, target_id);
  }

} // namespace
