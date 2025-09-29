#pragma once

#include <string>
#include <fstream>
#include <mutex>
#include <memory>
#include <chrono>
#include <iostream>

namespace ccsim::core {

  enum class LogLevel {
    DEBUG,
    INFO,
    WARNING,
    ERROR
  };

  class Logger {
  public:
    static Logger& instance();
    
    void set_level(LogLevel level);
    void set_output_file(const std::string& filename);
    
    void debug(const std::string& message);
    void info(const std::string& message);
    void warning(const std::string& message);
    void error(const std::string& message);
    
    // Performance logging
    void log_performance(const std::string& operation, 
                        std::chrono::milliseconds duration);
    
  private:
    Logger() = default;
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;
    
    void log(LogLevel level, const std::string& message);
    std::string format_message(LogLevel level, const std::string& message);
    std::string get_timestamp();
    
    LogLevel current_level_{LogLevel::INFO};
    std::unique_ptr<std::ofstream> file_output_;
    std::mutex log_mutex_;
  };

  // RAII performance timer
  class PerformanceTimer {
  public:
    explicit PerformanceTimer(const std::string& operation);
    ~PerformanceTimer();
    
  private:
    std::string operation_;
    std::chrono::steady_clock::time_point start_time_;
  };

  // Macro for easy performance timing
  #define PERF_TIMER(operation) PerformanceTimer _timer(operation)

} // namespace ccsim::core
