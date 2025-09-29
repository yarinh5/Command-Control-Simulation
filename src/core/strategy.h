#pragma once

#include "domain/command.h"
#include "domain/unit.h"
#include <vector>
#include <memory>
#include <unordered_map>
#include <queue>

namespace ccsim::core {

  // Strategy interface for command dispatch policies
  class CommandDispatchStrategy {
  public:
    virtual ~CommandDispatchStrategy() = default;
    virtual std::vector<UnitId> select_targets(
      const std::vector<UnitId>& available_units,
      const domain::Command& command) = 0;
  };

  // Round-robin strategy - distributes commands evenly
  class RoundRobinStrategy : public CommandDispatchStrategy {
  public:
    RoundRobinStrategy() : current_index_(0) {}
    
    std::vector<UnitId> select_targets(
      const std::vector<UnitId>& available_units,
      const domain::Command& command) override;
    
  private:
    std::atomic<size_t> current_index_;
  };

  // Priority strategy - targets units based on priority/status
  class PriorityStrategy : public CommandDispatchStrategy {
  public:
    PriorityStrategy() = default;
    
    std::vector<UnitId> select_targets(
      const std::vector<UnitId>& available_units,
      const domain::Command& command) override;
    
    void set_unit_priority(const UnitId& unit_id, int priority);
    
  private:
    int get_priority_or_default(const UnitId& unit_id) const;
    std::unordered_map<UnitId, int, domain::UnitId::Hash> unit_priorities_;
  };

  // Broadcast strategy - sends to all available units
  class BroadcastStrategy : public CommandDispatchStrategy {
  public:
    std::vector<UnitId> select_targets(
      const std::vector<UnitId>& available_units,
      const domain::Command& command) override;
  };

  // Strategy factory
  class StrategyFactory {
  public:
    enum class StrategyType {
      ROUND_ROBIN,
      PRIORITY,
      BROADCAST
    };
    
    static std::unique_ptr<CommandDispatchStrategy> create_strategy(StrategyType type);
  };

  // Command queue with priority support
  class CommandQueue {
  public:
    struct CommandEntry {
      std::unique_ptr<domain::Command> command;
      int priority;
      std::chrono::steady_clock::time_point timestamp;
      
      CommandEntry(std::unique_ptr<domain::Command> cmd, int prio = 0)
        : command(std::move(cmd)), priority(prio),
          timestamp(std::chrono::steady_clock::now()) {}
      
      bool operator<(const CommandEntry& other) const {
        if (priority != other.priority) {
          return priority < other.priority; // Higher priority first
        }
        return timestamp > other.timestamp; // Earlier timestamp first
      }
    };
    
    void enqueue(std::unique_ptr<domain::Command> command, int priority = 0);
    std::unique_ptr<domain::Command> dequeue();
    bool empty() const;
    size_t size() const;
    
  private:
    std::priority_queue<CommandEntry> queue_;
    mutable std::mutex queue_mutex_;
  };

} // namespace ccsim::core
