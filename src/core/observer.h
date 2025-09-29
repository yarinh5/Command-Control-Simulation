#pragma once

#include "domain/telemetry.h"
#include "domain/command.h"
#include <memory>
#include <vector>
#include <functional>
#include <mutex>
#include <unordered_set>

namespace ccsim::core {

  // Observer interface for telemetry events
  class TelemetryObserver {
  public:
    virtual ~TelemetryObserver() = default;
    virtual void on_telemetry_received(const domain::TelemetryReport& report) = 0;
  };

  // Observer interface for command events
  class CommandObserver {
  public:
    virtual ~CommandObserver() = default;
    virtual void on_command_sent(const domain::Command& command) = 0;
    virtual void on_command_completed(const std::string& command_id, bool success) = 0;
  };

  // Observable telemetry manager
  class TelemetryObservable {
  public:
    void add_observer(std::shared_ptr<TelemetryObserver> observer) {
      std::lock_guard<std::mutex> lock(observers_mutex_);
      observers_.insert(observer);
    }
    
    void remove_observer(std::shared_ptr<TelemetryObserver> observer) {
      std::lock_guard<std::mutex> lock(observers_mutex_);
      observers_.erase(observer);
    }
    
    void notify_telemetry(const domain::TelemetryReport& report) {
      std::lock_guard<std::mutex> lock(observers_mutex_);
      for (auto& observer : observers_) {
        if (auto obs = observer.lock()) {
          obs->on_telemetry_received(report);
        }
      }
    }
    
  private:
    std::unordered_set<std::weak_ptr<TelemetryObserver>, std::owner_less<std::weak_ptr<TelemetryObserver>>> observers_;
    std::mutex observers_mutex_;
  };

  // Observable command manager
  class CommandObservable {
  public:
    void add_observer(std::shared_ptr<CommandObserver> observer) {
      std::lock_guard<std::mutex> lock(observers_mutex_);
      observers_.insert(observer);
    }
    
    void remove_observer(std::shared_ptr<CommandObserver> observer) {
      std::lock_guard<std::mutex> lock(observers_mutex_);
      observers_.erase(observer);
    }
    
    void notify_command_sent(const domain::Command& command) {
      std::lock_guard<std::mutex> lock(observers_mutex_);
      for (auto& observer : observers_) {
        if (auto obs = observer.lock()) {
          obs->on_command_sent(command);
        }
      }
    }
    
    void notify_command_completed(const std::string& command_id, bool success) {
      std::lock_guard<std::mutex> lock(observers_mutex_);
      for (auto& observer : observers_) {
        if (auto obs = observer.lock()) {
          obs->on_command_completed(command_id, success);
        }
      }
    }
    
  private:
    std::unordered_set<std::weak_ptr<CommandObserver>, std::owner_less<std::weak_ptr<CommandObserver>>> observers_;
    std::mutex observers_mutex_;
  };

  // Concrete observers for logging and monitoring
  class LoggingTelemetryObserver : public TelemetryObserver {
  public:
    void on_telemetry_received(const domain::TelemetryReport& report) override;
  };

  class LoggingCommandObserver : public CommandObserver {
  public:
    void on_command_sent(const domain::Command& command) override;
    void on_command_completed(const std::string& command_id, bool success) override;
  };

} // namespace ccsim::core
