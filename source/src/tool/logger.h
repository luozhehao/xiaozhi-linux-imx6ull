

#include <iostream>
#include <fstream>
#include <sstream>
#include <memory>
#include <mutex>
#include <iomanip>
#include <chrono>
#include <vector>

enum class LogLevel {
    INFO,
    WARNING,
    ERROR
};

class Logger {
private:
    std::vector<std::ostream*> outputStreams;
    std::unique_ptr<std::ofstream> fileStream;
    std::mutex logMutex;
    
    // 私有构造函数，固定使用log.txt作为文件名
    Logger() {
        // 总是添加终端输出
        outputStreams.push_back(&std::cout);
        
        // 固定使用log.txt作为日志文件名
        // fileStream = std:: std::make_unique<std::ofstream>(
        //     "log.txt", std::ios::out | std::ios::app);
        fileStream.reset(new std::ofstream("log.txt", std::ios::out | std::ios::app));
        if (fileStream->is_open()) {
            outputStreams.push_back(fileStream.get());
            std::cout << "Log file opened: log.txt" << std::endl;
        } else {
            std::cerr << "Failed to open log file: log.txt" << std::endl;
        }
    }

    // 获取当前时间字符串
    std::string getCurrentTime() {
        auto now = std::chrono::system_clock::now();
        auto time_t = std::chrono::system_clock::to_time_t(now);
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
            now.time_since_epoch()) % 1000;
        
        std::stringstream ss;
        ss << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S");
        ss << "." << std::setfill('0') << std::setw(3) << ms.count();
        return ss.str();
    }

    // 获取日志级别字符串
    std::string getLevelString(LogLevel level) {
        switch(level) {
            case LogLevel::INFO: return "INFO";
            case LogLevel::WARNING: return "WARNING";
            case LogLevel::ERROR: return "ERROR";
            default: return "UNKNOWN";
        }
    }

    // 获取带颜色的日志级别字符串（仅用于终端）
    std::string getColoredLevelString(LogLevel level) {
        switch(level) {
            case LogLevel::INFO: return "\033[32mINFO\033[0m";    // 绿色
            case LogLevel::WARNING: return "\033[33mWARNING\033[0m"; // 黄色
            case LogLevel::ERROR: return "\033[31mERROR\033[0m";   // 红色
            default: return "UNKNOWN";
        }
    }

    // 核心日志写入方法
    void writeLog(LogLevel level, const std::string& message) {
        std::lock_guard<std::mutex> lock(logMutex);
        
        std::string timestamp = getCurrentTime();
        std::string levelStr = getLevelString(level);
        std::string coloredLevelStr = getColoredLevelString(level);
        
        // 构建日志条目
        std::stringstream logEntry;
        logEntry << "[" << timestamp << "] "
                 << "[" << levelStr << "] "
                 << message;
        
        // 为终端构建带颜色的日志条目
        std::stringstream coloredLogEntry;
        coloredLogEntry << "[" << timestamp << "] "
                        << "[" << coloredLevelStr << "] "
                        << message;
        
        // 输出到所有流
        for (auto& stream : outputStreams) {
            if (stream == &std::cout) {
                // 对终端使用带颜色的输出
                *stream << coloredLogEntry.str() << std::endl;
            } else {
                // 对文件使用普通输出
                *stream << logEntry.str() << std::endl;
            }
            stream->flush();
        }
    }

public:
    // 禁止拷贝和赋值
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;
    
    // 获取单例实例（无参版本）
    static Logger& getInstance() {
        static Logger instance;
        return instance;
    }

    ~Logger() {
        if (fileStream && fileStream->is_open()) {
            fileStream->close();
        }
    }

    // 日志接口方法
    static void info(const std::string& message) {
        getInstance().writeLog(LogLevel::INFO, message);
    }

    static void warning(const std::string& message) {
        getInstance().writeLog(LogLevel::WARNING, message);
    }

    static void error(const std::string& message) {
        getInstance().writeLog(LogLevel::ERROR, message);
    }

    // 格式化日志方法
    template<typename... Args>
    static void info(const std::string& format, Args... args) {
        info(formatMessage(format, args...));
    }

    template<typename... Args>
    static void warning(const std::string& format, Args... args) {
        warning(formatMessage(format, args...));
    }

    template<typename... Args>
    static void error(const std::string& format, Args... args) {
        error(formatMessage(format, args...));
    }

private:
    // 格式化消息（类似printf，但更安全）
    template<typename... Args>
    static std::string formatMessage(const std::string& format, Args... args) {
        size_t size = snprintf(nullptr, 0, format.c_str(), args...) + 1;
        std::unique_ptr<char[]> buf(new char[size]);
        snprintf(buf.get(), size, format.c_str(), args...);
        return std::string(buf.get(), buf.get() + size - 1);
    }
};

// 使用示例
// int main() {
//     // 使用静态方法直接记录日志
//     Logger::info("Application started");
//     Logger::warning("Low memory warning");
//     Logger::error("File not found: %s", "config.ini");
    
//     // 使用格式化输出
//     int retryCount = 3;
//     Logger::info("Retry attempt %d of %d", 1, retryCount);
//     Logger::warning("Connection timeout after %d ms", 5000);
//     Logger::error("Failed to initialize module: %s with code %d", "network", -1);
    
//     return 0;
// }