#include "unit.h"

namespace ccsim::domain {

  // UnitId implementation is header-only
  
  // Unit implementation
  Unit::Unit(const UnitId& id, const Position& initial_pos)
    : id_(id), position_(initial_pos), status_(UnitStatus::IDLE),
      last_update_(std::chrono::steady_clock::now()) {}

} // namespace ccsim::domain
