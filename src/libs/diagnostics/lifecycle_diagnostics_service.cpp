#include "lifecycle_diagnostics_service.hpp"

#include <condition_variable>
#include <cstdio>
#include <mutex>
#include <thread>

#include <libs/core/v_file_service.h>
#include <libs/util/fs.h>
#include <spdlog/spdlog.h>

#include "spdlog_sinks/syncable_sink.hpp"

#include "watermark.hpp"

#if defined(_UNICODE) && defined(_WIN32)
#include <tchar.h>
#define sentry_options_set_database_path sentry_options_set_database_pathw
#define sentry_options_set_handler_path sentry_options_set_handler_pathw
#define sentry_options_add_attachment sentry_options_add_attachmentw
#else
#include <cstdlib>
#define _T(x) x
#define _tsystem std::system
#endif

#ifdef _WIN32
#include <libs/diagnostics/logging.hpp>
#include <libs/diagnostics/seh_extractor.hpp>
#endif

namespace
{
auto& getExecutableDir()
{
    static auto const executableDir = std::filesystem::path(fio->executable_directory());
    return executableDir;
}
auto& getLogsArchive()
{
    static auto const logsArchive = fs::GetLogsPath().replace_extension(".7z");
    return logsArchive;
}

#ifdef _WIN32  // FIX_LINUX 7za.exe
auto assembleArchiveCmd()
{
    constexpr auto archiverBin = "7za.exe";
    return _T("call \"") + (getExecutableDir() / archiverBin).native() + _T("\" a \"\\\\?\\") + getLogsArchive().native()
        + _T("\" \"\\\\?\\") + fs::GetLogsPath().native() + _T("\"");
}
#endif

void log_sentry(sentry_level_t level, const char* message, va_list args, void*)
{
    // TODO:
    /* spdlog::level::level_enum log_level = spdlog::level::critical;
    switch (level)
    {
    case SENTRY_LEVEL_DEBUG:
        log_level = spdlog::level::debug;
        break;
    case SENTRY_LEVEL_INFO:
        log_level = spdlog::level::info;
        break;
    case SENTRY_LEVEL_WARNING:
        log_level = spdlog::level::warn;
        break;
    case SENTRY_LEVEL_ERROR:
        log_level = spdlog::level::err;
        break;
    case SENTRY_LEVEL_FATAL:
        log_level = spdlog::level::critical;
        break;
    }

    char text[128];
    if (vsnprintf(text, std::size(text), message, args) > 0)
    {
        spdlog::log(log_level, text);
    }*/
}

}  // namespace

namespace storm::diag
{

class LoggingService final
{
public:
    ~LoggingService()
    {
        terminate();
    }

    void initialize()
    {
        if (terminate_) {
            terminate_ = false;

            static auto terminate_handler = std::get_terminate();
            std::set_terminate([] { terminate_handler(); });

            create_directories(fs::GetLogsPath());

            std::thread worker {[this] { loggingThread(); }};
            worker.detach();
        }
    }

    void terminate()
    {
        using namespace std::chrono_literals;

        if (!terminate_) {
            terminate_ = true;
            {
                std::lock_guard lock(mtx_);
                flushRequested_ = true;
            }
            cv_.notify_one();

            std::unique_lock lock(mtx_);
            cv_.wait_for(lock, 5s, [this] { return !flushRequested_; });
            spdlog::shutdown();
        }
    }

    void flushAsync()
    {
        {
            std::lock_guard lock(mtx_);
            flushRequested_ = true;
        }
        cv_.notify_one();
    }

private:
    std::mutex              mtx_;
    std::condition_variable cv_;
    bool                    flushRequested_ {false};
    std::atomic_bool        terminate_ {true};

    void flushAll(bool const terminate) const
    {
        spdlog::apply_all([terminate](std::shared_ptr<spdlog::logger> l) {
            l->flush();

            if (terminate) {
                for (auto& sink: l->sinks()) {
                    if (auto const syncable_sink = std::dynamic_pointer_cast<logging::sinks::syncable_sink>(sink)) {
                        syncable_sink->terminate_immediately();
                    }
                }
            }
        });
    }

    void loggingThread()
    {
        while (!terminate_) {
            std::unique_lock lock(mtx_);
            cv_.wait(lock, [this] { return flushRequested_; });

            flushAll(terminate_);

            flushRequested_ = false;
            cv_.notify_one();
        }
    }
};

LifecycleDiagnosticsService::LifecycleDiagnosticsService() : loggingService_(std::make_unique<LoggingService>()) {}

LifecycleDiagnosticsService::~LifecycleDiagnosticsService()
{
    terminate();
}

LifecycleDiagnosticsService::Guard LifecycleDiagnosticsService::initialize(bool const enableCrashReports)
{
    loggingService_->initialize();
    initialized_ = true;

    return Guard(*this);
}

void LifecycleDiagnosticsService::terminate() const
{
    loggingService_->terminate();
}

void LifecycleDiagnosticsService::notifyAfterRun() const
{
    static auto latestFlushTimePoint = std::chrono::steady_clock::now();

    if (auto const now = std::chrono::steady_clock::now();
        std::chrono::duration_cast<std::chrono::seconds>(now - latestFlushTimePoint) >= getLoggingFlushPeriod()) {
        loggingService_->flushAsync();
        latestFlushTimePoint = now;
    }
}

void LifecycleDiagnosticsService::setCrashInfoCollector(crash_info_collector f)
{
    collectCrashInfo_ = std::move(f);
}

sentry_value_t LifecycleDiagnosticsService::beforeCrash(sentry_ucontext_t const* uctx, sentry_value_t event, void* closure)
{
    auto const* self = static_cast<LifecycleDiagnosticsService*>(closure);

    // collect engine data
    if (self->collectCrashInfo_) { self->collectCrashInfo_(); }

#ifdef _WIN32
    // collect exception data
    if (uctx != nullptr) {
        if (seh_extractor const seh(&uctx->exception_ptrs); seh.is_abnormal()) {
            static auto logger = logging::getOrCreateLogger("exceptions");
            logger->set_pattern("%v");
            seh.sink([](char const* msg) { logger->trace(msg); });
        }
    }
#endif

    // terminate logging
    self->loggingService_->terminate();

#ifdef _WIN32  // FIX_LINUX 7za.exe
    // archive logs for sentry backend
    _tsystem(assembleArchiveCmd().c_str());
#endif

    return event;
}

}  // namespace storm::diag
