#pragma once

#include <string>
#include <chrono>
#include <memory>
#include <atomic>

namespace ccsim::domain {

  class UnitId {
  public:
    explicit UnitId(const std::string& id) : id_(id) {}
    
    const std::string& value() const { return id_; }
    
    bool operator==(const UnitId& other) const { return id_ == other.id_; }
    bool operator!=(const UnitId& other) const { return !(*this == other); }
    
    struct Hash {
      std::size_t operator()(const UnitId& unit_id) const {
        return std::hash<std::string>{}(unit_id.id_);
      }
    };
    
  private:
    std::string id_;
  };

  enum class UnitStatus {
    IDLE,
    MOVING,
    BUSY,
    ERROR,
    OFFLINE
  };

  struct Position {
    double x{0.0};
    double y{0.0};
    double z{0.0};
    
    bool operator==(const Position& other) const {
      return x == other.x && y == other.y && z == other.z;
    }
  };

  class Unit {
  public:
    Unit(const UnitId& id, const Position& initial_pos = {})
      : id_(id), position_(initial_pos), status_(UnitStatus::IDLE),
        last_update_(std::chrono::steady_clock::now()) {}
    
    const UnitId& id() const { return id_; }
    const Position& position() const { return position_; }
    UnitStatus status() const { return status_; }
    std::chrono::steady_clock::time_point last_update() const { return last_update_; }
    
    void set_position(const Position& pos) {
      position_ = pos;
      update_timestamp();
    }
    
    void set_status(UnitStatus status) {
      status_ = status;
      update_timestamp();
    }
    
    bool is_online() const {
      auto now = std::chrono::steady_clock::now();
      auto timeout = std::chrono::seconds(30);
      return (now - last_update_) < timeout;
    }
    
  private:
    void update_timestamp() {
      last_update_ = std::chrono::steady_clock::now();
    }
    
    UnitId id_;
    Position position_;
    std::atomic<UnitStatus> status_;
    std::chrono::steady_clock::time_point last_update_;
  };

  using UnitPtr = std::shared_ptr<Unit>;

} // namespace ccsim::domain
