#include "command.h"

namespace ccsim::domain {

  // Command implementation
  Command::Command(const UnitId& target_id, CommandType type, const nlohmann::json& payload)
    : target_id_(target_id), type_(type), payload_(payload),
      timestamp_(std::chrono::steady_clock::now()),
      id_(generate_id()) {}

  std::string Command::generate_id() {
    static std::atomic<uint64_t> counter{0};
    return "cmd_" + std::to_string(counter.fetch_add(1));
  }

} // namespace ccsim::domain
