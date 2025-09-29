#include "logger.h"
#include <iomanip>
#include <sstream>

namespace ccsim::core {

  Logger& Logger::instance() {
    static Logger instance;
    return instance;
  }

  void Logger::set_level(LogLevel level) {
    std::lock_guard<std::mutex> lock(log_mutex_);
    current_level_ = level;
  }

  void Logger::set_output_file(const std::string& filename) {
    std::lock_guard<std::mutex> lock(log_mutex_);
    file_output_ = std::make_unique<std::ofstream>(filename, std::ios::app);
  }

  void Logger::debug(const std::string& message) {
    log(LogLevel::DEBUG, message);
  }

  void Logger::info(const std::string& message) {
    log(LogLevel::INFO, message);
  }

  void Logger::warning(const std::string& message) {
    log(LogLevel::WARNING, message);
  }

  void Logger::error(const std::string& message) {
    log(LogLevel::ERROR, message);
  }

  void Logger::log_performance(const std::string& operation, 
                              std::chrono::milliseconds duration) {
    std::string message = operation + " took " + std::to_string(duration.count()) + "ms";
    log(LogLevel::INFO, message);
  }

  void Logger::log(LogLevel level, const std::string& message) {
    if (level < current_level_) {
      return;
    }
    
    std::string formatted = format_message(level, message);
    
    std::lock_guard<std::mutex> lock(log_mutex_);
    
    // Output to console
    if (level >= LogLevel::WARNING) {
      std::cerr << formatted << std::endl;
    } else {
      std::cout << formatted << std::endl;
    }
    
    // Output to file if configured
    if (file_output_ && file_output_->is_open()) {
      *file_output_ << formatted << std::endl;
      file_output_->flush();
    }
  }

  std::string Logger::format_message(LogLevel level, const std::string& message) {
    std::stringstream ss;
    
    // Timestamp
    ss << "[" << get_timestamp() << "] ";
    
    // Log level
    switch (level) {
      case LogLevel::DEBUG:
        ss << "[DEBUG] ";
        break;
      case LogLevel::INFO:
        ss << "[INFO]  ";
        break;
      case LogLevel::WARNING:
        ss << "[WARN]  ";
        break;
      case LogLevel::ERROR:
        ss << "[ERROR] ";
        break;
    }
    
    // Message
    ss << message;
    
    return ss.str();
  }

  std::string Logger::get_timestamp() {
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
      now.time_since_epoch()) % 1000;
    
    std::stringstream ss;
    ss << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S");
    ss << "." << std::setfill('0') << std::setw(3) << ms.count();
    
    return ss.str();
  }

  // PerformanceTimer implementation
  PerformanceTimer::PerformanceTimer(const std::string& operation)
    : operation_(operation), start_time_(std::chrono::steady_clock::now()) {}

  PerformanceTimer::~PerformanceTimer() {
    auto end_time = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
      end_time - start_time_);
    
    Logger::instance().log_performance(operation_, duration);
  }

} // namespace ccsim::core
