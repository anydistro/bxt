/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2025 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
// #include <filesystem>
// #include <memory>
// #include <string_view>
// #include <utility>
// #include <vector>

// #include <boost/di.hpp>
// #include <coro/io_scheduler.hpp>
// #include <coro/sync_wait.hpp>
// #include <drogon/HttpAppFramework.h>
// #include <drogon/HttpTypes.h>
// #include <fmt/ranges.h>
// #include <spdlog/async.h>
// #include <spdlog/common.h>
// #include <spdlog/logger.h>
// #include <spdlog/sinks/basic_file_sink.h>
// #include <spdlog/sinks/stdout_color_sinks.h>
// #include <spdlog/spdlog.h>
// #include <sqlgen/mysql/connect.hpp>
// #include <sqlgen/mysql/Credentials.hpp>
// #include <trantor/utils/Logger.h>

// // Modules
// #include "adapters/sqlgen/SqlgenSettings.hpp"
// #include "adapters/sqlgen/SqlgenUnitOfWorkFactory.hpp"
// #include "features/Users/UsersModule.hpp"

// // Shared
// #include "infra/drogon/middleware/LogMiddleware.hpp"
// #include "infra/logging/Logging.hpp"
// #include "infra/logging/FileFormatter.hpp"
// #include "infra/logging/PrettyFormatter.hpp"
// #include "core/di/Concepts.hpp"
// #include "signal/SignalHandler.hpp"
// #include "utils/MemoryLiterals.hpp"

// namespace di = boost::di;

// namespace bxt {

// namespace detail {

//     template<typename TInjector, typename TTuple, std::size_t... I>
//     void create_controllers(TInjector& injector,
//                             std::vector<std::shared_ptr<drogon::HttpControllerBase>>&
//                             controllers, std::index_sequence<I...>) {
//         // Use fold expression with expanded indices
//         (([&]() {
//              using T = std::tuple_element_t<I, TTuple>;
//              auto controller = injector.template create<std::shared_ptr<T>>();
//              controllers.push_back(controller);
//              drogon::app().registerController(controller);
//          }()),
//          ...);
//     }

//     // Helper function to infer tuple size and create the right index sequence
//     template<typename TInjector, typename TTuple>
//     void create_controllers(TInjector& injector,
//                             std::vector<std::shared_ptr<drogon::HttpControllerBase>>&
//                             controllers) {
//         constexpr size_t tuple_size = std::tuple_size_v<TTuple>;
//         create_controllers<TInjector, TTuple>(injector, controllers,
//                                               std::make_index_sequence<tuple_size> {});
//     }

// } // namespace detail

// using namespace MemoryLiterals;
// template<Module... TModuleFactories> class Application {
//     constexpr static auto client_max_body_size = 256_MB;
//     constexpr static auto client_max_memory_body_size = 1_MB;

//     constexpr static uint16_t port = 8080;
//     constexpr static double eventloop_duration = 0.1;

// public:
//     Application() {
//         setup_logger();
//         setup_signal_safe_tracing(); // Setup signal-safe crash tracing
//         bxt::logi("🚀 Initializing application...");

//         // Create I/O scheduler
//         m_scheduler = coro::io_scheduler::make_shared({
//             .thread_strategy = coro::io_scheduler::thread_strategy_t::manual,
//         });

//         // Create base injector with common services
//         auto injector = di::make_injector(
//             di::bind<coro::io_scheduler>().to(m_scheduler),
//             di::bind<adapters::sqlgen::SqlgenSettings>()
//                 .to(adapters::sqlgen::SqlgenSettings {.connection_string = "./bxt.db"})
//                 .in(di::singleton));

//         if (!std::filesystem::exists("box.yml")) {
//             bxt::logw("⚠️📦 Box schema file is not found, no repositories will be registered");
//         } else {
//             bxt::logi("📦 Loading schema from {}...",
//                       std::filesystem::absolute("box.yml").string());
//         }

//         if (!std::filesystem::exists("web/")) {
//             bxt::logw("⚠️🕸 Web UI is not found in {}, bxt-web is not available",
//                       std::filesystem::absolute("web/").string());
//         } else {
//             bxt::logi("🕸 Web UI is available at /index.html");
//         }

//         if (!std::filesystem::exists("settings.toml")) {
//             bxt::logf("⚠️📝 Settings file is not found, aborting...");
//             std::abort();
//         } else {
//             bxt::logi("📝 Loading settings from {}...",
//                       std::filesystem::absolute("settings.toml").string());
//         }

//         // Register each module
//         (register_module<TModuleFactories>(injector), ...);
//         bxt::logi_s("📦 All modules registered", modules());
//     }

//     std::vector<std::string_view> modules() const {
//         return {TModuleFactories::name()...};
//     }

//     int run([[maybe_unused]] int argc, [[maybe_unused]] char** argv) {
//         bxt::logi("🎯 Starting application...");

//         // Setup Drogon
//         auto& app = drogon::app()
//                         .setDocumentRoot("./web/")
//                         .enableCompressedRequest()
//                         .addListener("0.0.0.0", port)
//                         .setUploadPath("/tmp/bxt/")
//                         .setClientMaxBodySize(client_max_body_size)
//                         .setClientMaxMemoryBodySize(client_max_memory_body_size);

//         app.registerBeginningAdvice(
//             []() { bxt::logi("📡 Application is listening on port {}", port); });

//         // app.registerMiddleware(std::make_shared<bxt::middleware::LogMiddleware>());

//         // Setup scheduler processing
//         app.getLoop()->runEvery(eventloop_duration, [this]() { m_scheduler->process_events(); });

//         bxt::logi("🛠️🐉 Drogon framework configured");

//         // Start the application
//         app.run();

//         return 0;
//     }

// private:
//     // Store controllers returned by modules
//     std::vector<std::shared_ptr<drogon::HttpControllerBase>> m_controllers;
//     // Store module instances to maintain their lifetime
//     std::vector<std::shared_ptr<void>> m_modules;
//     // Schedule for async event processing
//     std::shared_ptr<coro::io_scheduler> m_scheduler;

//     void setup_logger() {
//         spdlog::init_thread_pool(10240, 2);
//         spdlog::flush_every(std::chrono::seconds(3));

//         // Create console sink with pretty formatter
//         auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
//         console_sink->set_formatter(std::make_unique<bxt::PrettyFormatter>());

//         std::filesystem::create_directories("./logs");

//         auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>("logs/bxt.log",
//         true); file_sink->set_formatter(std::make_unique<bxt::FileFormatter>());
//         // Create logger with both sinks
//         std::vector<spdlog::sink_ptr> sinks {console_sink, file_sink};

//         auto logger = std::make_shared<spdlog::async_logger>("bxt", sinks.begin(), sinks.end(),
//                                                              spdlog::thread_pool(),
//                                                              spdlog::async_overflow_policy::block);

//         // Set as default logger
//         spdlog::register_logger(logger);
//         spdlog::set_default_logger(logger);

//         // Configure Drogon's logging to use spdlog
//         auto drogon_console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
//         drogon_console_sink->set_formatter(std::make_unique<bxt::PrettyFormatter>("🐉💬 "));

//         sinks = {drogon_console_sink, file_sink};
//         auto drogon_logger = std::make_shared<spdlog::async_logger>(
//             "drogon", sinks.begin(), sinks.end(), spdlog::thread_pool(),
//             spdlog::async_overflow_policy::block);

//         trantor::Logger::enableSpdLog(drogon_logger);
//     }

//     template<Module TModuleFactory> auto register_module(auto& injector) {
//         auto module = TModuleFactory {}();
//         m_modules.push_back(module);

//         auto new_injector = module->configure(std::move(injector));

//         // Create controllers using the tuple of types from the module
//         using ModuleType = std::remove_reference_t<decltype(*module)>;
//         using ControllerTypes = typename ModuleType::controller_types;
//         detail::create_controllers<decltype(new_injector), ControllerTypes>(new_injector,
//                                                                             m_controllers);

//         return new_injector;
//     }
// };
// } // namespace bxt

#include <iostream>

#include <sqlgen.hpp>
#include <sqlgen/begin_transaction.hpp>
#include <sqlgen/sqlite/connect.hpp>

#include "signal/SignalHandler.hpp"

int main([[maybe_unused]] int argc, [[maybe_unused]] char** argv) {
    bxt::setup_signal_safe_tracing();

    // bxt::Application<bxt::Users::UsersModule> app;

    // return app.run(argc, argv); 
    struct Person {
        std::string first_name;
        std::string last_name;
        uint32_t age;
    };
    auto const people =
        std::vector<Person>({Person {.first_name = "Homer", .last_name = "Simpson", .age = 45},
                             Person {.first_name = "Marge", .last_name = "Simpson", .age = 42}});

    auto conn = ::sqlgen::sqlite::connect();

    auto const result = ::sqlgen::write(conn, people);

    if (!result) {
        std::cerr << "Error: " << result.error().what() << std::endl;
    }
}
