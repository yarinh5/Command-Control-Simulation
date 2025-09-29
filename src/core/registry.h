#pragma once

#include "domain/unit.h"
#include <unordered_map>
#include <mutex>
#include <memory>
#include <vector>

namespace ccsim::core {

  // Singleton registry for managing units
  class UnitRegistry {
  public:
    static UnitRegistry& instance();
    
    // Unit management
    void register_unit(std::shared_ptr<domain::Unit> unit);
    void unregister_unit(const domain::UnitId& unit_id);
    
    std::shared_ptr<domain::Unit> get_unit(const domain::UnitId& unit_id);
    std::vector<domain::UnitId> get_all_unit_ids() const;
    std::vector<std::shared_ptr<domain::Unit>> get_all_units() const;
    
    // Online/offline tracking
    std::vector<domain::UnitId> get_online_units() const;
    std::vector<domain::UnitId> get_offline_units() const;
    
    // Statistics
    size_t total_units() const;
    size_t online_units() const;
    size_t offline_units() const;
    
    // Unit status updates
    void update_unit_status(const domain::UnitId& unit_id, domain::UnitStatus status);
    void update_unit_position(const domain::UnitId& unit_id, const domain::Position& position);
    
  private:
    UnitRegistry() = default;
    UnitRegistry(const UnitRegistry&) = delete;
    UnitRegistry& operator=(const UnitRegistry&) = delete;
    
    mutable std::mutex units_mutex_;
    std::unordered_map<domain::UnitId, std::shared_ptr<domain::Unit>, domain::UnitId::Hash> units_;
  };

} // namespace ccsim::core
