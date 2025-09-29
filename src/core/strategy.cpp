#include "strategy.h"
#include <algorithm>
#include <random>

namespace ccsim::core {

  // RoundRobinStrategy implementation
  std::vector<domain::UnitId> RoundRobinStrategy::select_targets(
    const std::vector<domain::UnitId>& available_units,
    const domain::Command& command) {
    
    if (available_units.empty()) {
      return {};
    }
    
    std::vector<domain::UnitId> result;
    
    // For MOVE commands, select one unit
    if (command.type() == domain::CommandType::MOVE) {
      size_t index = current_index_.fetch_add(1) % available_units.size();
      result.push_back(available_units[index]);
    }
    // For REPORT commands, select all units
    else if (command.type() == domain::CommandType::REPORT) {
      result = available_units;
    }
    // For other commands, select one unit
    else {
      size_t index = current_index_.fetch_add(1) % available_units.size();
      result.push_back(available_units[index]);
    }
    
    return result;
  }

  // PriorityStrategy implementation
  std::vector<domain::UnitId> PriorityStrategy::select_targets(
    const std::vector<domain::UnitId>& available_units,
    const domain::Command& command) {
    
    if (available_units.empty()) {
      return {};
    }
    
    // Sort units by priority (higher priority first)
    std::vector<domain::UnitId> sorted_units = available_units;
    std::sort(sorted_units.begin(), sorted_units.end(),
      [this](const domain::UnitId& a, const domain::UnitId& b) {
        int priority_a = get_priority_or_default(a);
        int priority_b = get_priority_or_default(b);
        return priority_a > priority_b;
      });
    
    std::vector<domain::UnitId> result;
    
    // For MOVE commands, select highest priority unit
    if (command.type() == domain::CommandType::MOVE) {
      result.push_back(sorted_units[0]);
    }
    // For REPORT commands, select top 3 priority units
    else if (command.type() == domain::CommandType::REPORT) {
      size_t count = std::min(size_t(3), sorted_units.size());
      result.assign(sorted_units.begin(), sorted_units.begin() + count);
    }
    // For other commands, select highest priority unit
    else {
      result.push_back(sorted_units[0]);
    }
    
    return result;
  }

  void PriorityStrategy::set_unit_priority(const domain::UnitId& unit_id, int priority) {
    unit_priorities_[unit_id] = priority;
  }

  int PriorityStrategy::get_priority_or_default(const domain::UnitId& unit_id) const {
    auto it = unit_priorities_.find(unit_id);
    return (it != unit_priorities_.end()) ? it->second : 0;
  }

  // BroadcastStrategy implementation
  std::vector<domain::UnitId> BroadcastStrategy::select_targets(
    const std::vector<domain::UnitId>& available_units,
    const domain::Command& command) {
    
    return available_units;
  }

  // StrategyFactory implementation
  std::unique_ptr<CommandDispatchStrategy> StrategyFactory::create_strategy(StrategyType type) {
    switch (type) {
      case StrategyType::ROUND_ROBIN:
        return std::make_unique<RoundRobinStrategy>();
      case StrategyType::PRIORITY:
        return std::make_unique<PriorityStrategy>();
      case StrategyType::BROADCAST:
        return std::make_unique<BroadcastStrategy>();
      default:
        return std::make_unique<RoundRobinStrategy>();
    }
  }

  // CommandQueue implementation
  void CommandQueue::enqueue(std::unique_ptr<domain::Command> command, int priority) {
    std::lock_guard<std::mutex> lock(queue_mutex_);
    queue_.emplace(std::move(command), priority);
  }

  std::unique_ptr<domain::Command> CommandQueue::dequeue() {
    std::lock_guard<std::mutex> lock(queue_mutex_);
    if (queue_.empty()) {
      return nullptr;
    }
    
    auto entry = std::move(const_cast<CommandEntry&>(queue_.top()));
    queue_.pop();
    return std::move(entry.command);
  }

  bool CommandQueue::empty() const {
    std::lock_guard<std::mutex> lock(queue_mutex_);
    return queue_.empty();
  }

  size_t CommandQueue::size() const {
    std::lock_guard<std::mutex> lock(queue_mutex_);
    return queue_.size();
  }

} // namespace ccsim::core
