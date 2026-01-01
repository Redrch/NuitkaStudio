//
// Created by redrch on 2025/12/19.
//

#ifndef NUITKASTUDIO_LOGGER_H
#define NUITKASTUDIO_LOGGER_H

#include <QString>
#include <QMessageLogContext>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/async.h>

#ifdef _WIN32
#include <windows.h>
#endif

class Logger {
public:
    struct Config {
        bool async = true;  // if async
        size_t thread_pool_queue_size = 8192;  // 异步队列大小
        size_t thread_pool_thread_count = 1;  // 线程数
        std::string file_path = "logs/app.log";  // 日志文件路径
        size_t rotate_size = 1024 * 1024 * 64;  // 日志轮转大小 64Mib
        int rotate_count = 10;  // 轮转个数
        std::string pattern = "[%Y-%m-%d %H:%M:%S.%e] [%l] [%t] %v";  // 日志输出格式
        spdlog::level::level_enum level = spdlog::level::level_enum::debug;  // 日志等级
    };

    explicit Logger(const Config& config);
    ~Logger();

    void init(const Config& config);
    void shutdown();

    static void installQtMessageHandler();
    static void uninstallQtMessageHandler();

    static void debug(const QString& msg);
    static void info(const QString& msg);
    static void warn(const QString& msg);
    static void error(const QString& msg);


private:
    static void qtMessageHandler(QtMsgType type, const QMessageLogContext &ctx, const QString &msg);
};


#endif //NUITKASTUDIO_LOGGER_H
