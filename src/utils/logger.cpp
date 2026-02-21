//
// Created by redrch on 2025/12/19.
//

#include "logger.h"

Logger::Logger(const Logger::Config &config) {
    this->init(config);
}

Logger::~Logger() {
    this->shutdown();
}

void Logger::init(const Logger::Config &config) {
#ifdef _WIN32
    SetConsoleCP(CP_UTF8);
    SetConsoleOutputCP(CP_UTF8);
#endif

    spdlog::drop("app");
    if (config.async) {
        spdlog::init_thread_pool(config.thread_pool_queue_size, config.thread_pool_thread_count);
    }

    auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    auto file_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
            config.file_path, config.rotate_size, config.rotate_count);

    std::vector<spdlog::sink_ptr> sinks{console_sink, file_sink};
    if (config.async) {
        auto async_logger = std::make_shared<spdlog::async_logger>(
                "app", sinks.begin(), sinks.end(), spdlog::thread_pool(),
                spdlog::async_overflow_policy::block
        );
        spdlog::register_logger(async_logger);
        spdlog::set_default_logger(async_logger);
    } else {
        auto logger = std::make_shared<spdlog::logger>(
                "app", sinks.begin(), sinks.end()
        );
        spdlog::register_logger(logger);
        spdlog::set_default_logger(logger);
    }

    spdlog::set_pattern(config.pattern);
    spdlog::set_level(config.level);
    spdlog::flush_on(spdlog::level::level_enum::info);
}

void Logger::shutdown() {
    spdlog::shutdown();
}

void Logger::installQtMessageHandler() {
    qInstallMessageHandler(&Logger::qtMessageHandler);
}

void Logger::uninstallQtMessageHandler() {
    qInstallMessageHandler(nullptr);
}

void Logger::qtMessageHandler(QtMsgType type, const QMessageLogContext &ctx, const QString &msg) {
    auto logger = spdlog::get("nuitka-studio");
    if (!logger) return;
    std::string text = msg.toUtf8().toStdString();
    switch (type) {
        case QtDebugMsg:    logger->debug("{} ({}:{})", text, ctx.file ? ctx.file : "", ctx.line); break;
        case QtInfoMsg:     logger->info("{} ({}:{})",  text, ctx.file ? ctx.file : "", ctx.line); break;
        case QtWarningMsg:  logger->warn("{} ({}:{})",  text, ctx.file ? ctx.file : "", ctx.line); break;
        case QtCriticalMsg: logger->error("{} ({}:{})", text, ctx.file ? ctx.file : "", ctx.line); break;
        case QtFatalMsg:    logger->critical("{} ({}:{})", text, ctx.file ? ctx.file : "", ctx.line); abort();
    }
}

void Logger::debug(const QString& msg) {
    spdlog::debug(msg.toStdString());
}

void Logger::info(const QString& msg) {
    spdlog::info(msg.toStdString());
}

void Logger::warn(const QString& msg) {
    spdlog::warn(msg.toStdString());
}

void Logger::error(const QString& msg) {
    spdlog::error(msg.toStdString());
}