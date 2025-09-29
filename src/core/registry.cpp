#include "registry.h"
#include "logger.h"

namespace ccsim::core {

  UnitRegistry& UnitRegistry::instance() {
    static UnitRegistry instance;
    return instance;
  }

  void UnitRegistry::register_unit(std::shared_ptr<domain::Unit> unit) {
    std::lock_guard<std::mutex> lock(units_mutex_);
    units_[unit->id()] = unit;
    
    auto& logger = Logger::instance();
    logger.info("Unit registered: " + unit->id().value());
  }

  void UnitRegistry::unregister_unit(const domain::UnitId& unit_id) {
    std::lock_guard<std::mutex> lock(units_mutex_);
    auto it = units_.find(unit_id);
    if (it != units_.end()) {
      units_.erase(it);
      
      auto& logger = Logger::instance();
      logger.info("Unit unregistered: " + unit_id.value());
    }
  }

  std::shared_ptr<domain::Unit> UnitRegistry::get_unit(const domain::UnitId& unit_id) {
    std::lock_guard<std::mutex> lock(units_mutex_);
    auto it = units_.find(unit_id);
    return (it != units_.end()) ? it->second : nullptr;
  }

  std::vector<domain::UnitId> UnitRegistry::get_all_unit_ids() const {
    std::lock_guard<std::mutex> lock(units_mutex_);
    std::vector<domain::UnitId> ids;
    ids.reserve(units_.size());
    
    for (const auto& pair : units_) {
      ids.push_back(pair.first);
    }
    
    return ids;
  }

  std::vector<std::shared_ptr<domain::Unit>> UnitRegistry::get_all_units() const {
    std::lock_guard<std::mutex> lock(units_mutex_);
    std::vector<std::shared_ptr<domain::Unit>> units;
    units.reserve(units_.size());
    
    for (const auto& pair : units_) {
      units.push_back(pair.second);
    }
    
    return units;
  }

  std::vector<domain::UnitId> UnitRegistry::get_online_units() const {
    std::lock_guard<std::mutex> lock(units_mutex_);
    std::vector<domain::UnitId> online_units;
    
    for (const auto& pair : units_) {
      if (pair.second->is_online()) {
        online_units.push_back(pair.first);
      }
    }
    
    return online_units;
  }

  std::vector<domain::UnitId> UnitRegistry::get_offline_units() const {
    std::lock_guard<std::mutex> lock(units_mutex_);
    std::vector<domain::UnitId> offline_units;
    
    for (const auto& pair : units_) {
      if (!pair.second->is_online()) {
        offline_units.push_back(pair.first);
      }
    }
    
    return offline_units;
  }

  size_t UnitRegistry::total_units() const {
    std::lock_guard<std::mutex> lock(units_mutex_);
    return units_.size();
  }

  size_t UnitRegistry::online_units() const {
    return get_online_units().size();
  }

  size_t UnitRegistry::offline_units() const {
    return get_offline_units().size();
  }

  void UnitRegistry::update_unit_status(const domain::UnitId& unit_id, domain::UnitStatus status) {
    auto unit = get_unit(unit_id);
    if (unit) {
      unit->set_status(status);
    }
  }

  void UnitRegistry::update_unit_position(const domain::UnitId& unit_id, const domain::Position& position) {
    auto unit = get_unit(unit_id);
    if (unit) {
      unit->set_position(position);
    }
  }

} // namespace ccsim::core
