/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2022 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */

#include "core/application/dtos/PackageSectionDTO.h"
#include "core/application/services/CompareService.h"
#include "core/application/services/DeploymentService.h"
#include "core/application/services/SectionService.h"
#include "core/domain/entities/User.h"
#include "core/domain/repositories/UnitOfWorkBase.h"
#include "core/domain/value_objects/Name.h"
#include "core/domain/value_objects/Permission.h"
#include "coro/io_scheduler.hpp"
#include "coro/sync_wait.hpp"
#include "di.h"
#include "drogon/drogon_callbacks.h"
#include "drogon/HttpResponse.h"
#include "drogon/utils/FunctionTraits.h"
#include "events.h"
#include "presentation/cli-controllers/DeploymentOptions.h"
#include "presentation/JwtOptions.h"
#include "presentation/web-controllers/SectionController.h"
#include "utilities/configuration/Configuration.h"
#include "utilities/Error.h"
#include "utilities/errors/DatabaseError.h"

#include <boost/log/trivial.hpp>
#include <boost/log/utility/setup.hpp>
#include <cstdlib>
#include <drogon/HttpAppFramework.h>
#include <filesystem>
#include <kangaru/debug.hpp>
#include <lmdbxx/lmdb++.h>
#include <memory>
#include <system_error>
#include <toml++/toml.h>
#include <vector>

void setup_logger() {
    using namespace boost::log;

    add_common_attributes();
    add_console_log(std::cout,
                    boost::log::keywords::format = "[%TimeStamp%][%Severity%]: %Message%",
                    boost::log::keywords::auto_flush = true);
    add_file_log("journal.log",
                 boost::log::keywords::format = "[%TimeStamp%][%Severity%]: %Message%",
                 boost::log::keywords::auto_flush = true);
}

toml::table setup_toml_configuration(std::filesystem::path const& config_path) {
    if (!std::filesystem::exists(config_path)) {
        std::ofstream stream {config_path};
    }

    auto result = toml::parse_file(config_path.string());

    if (!result) {
        return toml::table {};
    }

    return result.table();
}

// Fill the dependency injection container
void setup_di_container(kgr::container& container) {
    using namespace bxt;

    auto scheduler = coro::io_scheduler::make_shared({
        .thread_strategy = coro::io_scheduler::thread_strategy_t::manual,
    });

    container.emplace<di::Utilities::IOScheduler>(scheduler);

    container.service<di::Utilities::EventBus>();

    container.service<di::Infrastructure::EventLogger>();

    container.service<di::Utilities::EventBusDispatcher>();

    container.emplace<di::Utilities::Configuration>(setup_toml_configuration("config.toml"));

    // Invoke all options structures to deserialize their values
    container.invoke<di::Utilities::Configuration, di::Utilities::LMDB::LMDBOptions,
                     di::Persistence::Box::BoxOptions, di::Presentation::JwtOptions,
                     di::Presentation::DeploymentOptions>(
        [](auto& configuration, auto& lmdb_options, auto& box_options, auto& jwt_options,
           auto& deployment_options) {
            lmdb_options.deserialize(configuration);
            box_options.deserialize(configuration);
            jwt_options.deserialize(configuration);
            deployment_options.deserialize(configuration);
        });

    // Parse the repository schema from a YAML file and extend the parser with
    // custom options
    container.invoke<di::Utilities::RepoSchema::Parser, di::Infrastructure::ArchRepoOptions,
                     di::Persistence::Box::PoolOptions>(
        [](auto& parser, auto& arch_repo_options, auto& pool_options) {
            parser.extend(&arch_repo_options);
            parser.extend(&pool_options);

            parser.parse("./box.yml");
        });

    container.invoke<di::Utilities::LMDB::Environment, di::Utilities::LMDB::LMDBOptions>(
        [](auto lmdbenv, auto& options) {
            lmdbenv->env().set_mapsize(50UL * 1024UL * 1024UL * 1024UL);
            lmdbenv->env().set_max_dbs(128);

            std::error_code ec;
            if (std::filesystem::create_directories(options.lmdb_path, ec); ec.value()) {
                logf("Cannot create LMDB folder. The error is \"{}\". Exiting.", ec.message());
                exit(1);
            }

            try {
                lmdbenv->env().open(options.lmdb_path.c_str(), 0, 0664);
            } catch (lmdb::error const& er) {
                logf("Cannot open LMDB database. The error is \"{}\". Exiting.", er.what());
                exit(1);
            }
        });

    // Register various repositories and services related to persistence and
    // business logic.
    container.service<di::Persistence::SectionRepository>();
    container.emplace<di::Persistence::UserRepository>(std::string("bxt::Users"));
    container.emplace<di::Persistence::SyncLogEntryRepository>(std::string("bxt::SyncLogEntries"));

    container.emplace<di::Persistence::CommitLogEntryRepository>(
        std::string("bxt::CommitLogEntries"));

    container.emplace<di::Persistence::DeployLogEntryRepository>(
        std::string("bxt::DeployLogEntries"));

    container.emplace<di::Persistence::UnitOfWorkFactory>();

    container.emplace<di::Persistence::Box::Pool>();
    container.emplace<di::Persistence::Box::LMDBPackageStore>("bxt::Box");

    container.emplace<di::Persistence::Box::AlpmDBExporter>();

    container.service<di::Persistence::Box::BoxRepository>();

    container.invoke<di::Persistence::Box::BoxRepository, di::Persistence::Box::Pool>(
        [](auto& box_repo, auto& pool) { pool.count_links(box_repo); });

    container.service<di::Core::Application::AuthService>();
    container.service<di::Core::Application::PermissionService>();

    container.service<di::Infrastructure::PackageService>();
    container.service<di::Infrastructure::DeploymentService>();
    container.service<di::Infrastructure::WSController>();

    container.service<di::Infrastructure::ArchRepoSyncService>();
    container.service<di::Core::Application::CompareService>();
}

void setup_controllers(drogon::HttpAppFramework& app, kgr::container& container) {
    using namespace bxt::di::Presentation;

    app.registerController(container.service<DeploymentController>())
        .registerController(container.service<PackageController>())
        .registerController(container.service<CompareController>())
        .registerController(container.service<AuthController>())
        .registerController(container.service<UserController>())
        .registerController(container.service<LogController>())
        .registerController(container.service<SectionController>())
        .registerController(container.service<bxt::di::Infrastructure::WSController>())
        .registerFilter(container.service<JwtFilter>());
}

void setup_scheduler(auto& app, auto scheduler, auto eventbus) {
    app.getLoop()->runEvery(1.0, [scheduler, eventbus]() {
        scheduler->process_events();
        eventbus->process();
    });
}

void setup_defaults(kgr::container& container) {
    using namespace bxt;
    auto& unit_of_work_factory = container.service<di::Core::Domain::UnitOfWorkBaseFactory>();
    auto uow = coro::sync_wait(unit_of_work_factory(true));

    auto& repository = container.service<di::Core::Domain::UserRepository>();

    auto const users = coro::sync_wait(repository.all_async(uow));

    if (!users.has_value()) {
        BOOST_LOG_TRIVIAL(error) << users.error().what();
        abort();
    }

    if (users->empty()) {
        User default_user(Name("default"), "ILoveMacarons");
        default_user.set_permissions({Permission("*")});

        auto const add_result = coro::sync_wait(repository.save_async(default_user, uow));
        if (!add_result.has_value()) {
            bxt::loge(add_result.error().what());
            coro::sync_wait(uow->rollback_async());
            abort();
        }

        auto const commit_result = coro::sync_wait(uow->commit_async());
        if (!commit_result.has_value()) {
            bxt::loge(commit_result.error().what());
            abort();
        }
    }
}

int main() {
    setup_logger();

    kgr::container container;

    setup_di_container(container);

    setup_defaults(container);

    auto const serveFrontendAdvice = [](drogon::HttpRequestPtr const& req,
                                        drogon::AdviceCallback&& acb,
                                        drogon::AdviceChainCallback&& accb) {
        if (req->path() == "/swagger") {
            auto const indexPath =
                fmt::format("{}/swagger/index.html", drogon::app().getDocumentRoot());

            acb(drogon::HttpResponse::newFileResponse(indexPath));
            return;
        }

        if (req->path().starts_with("/api/")) {
            accb();
            return;
        }

        auto const resource = drogon::app().getDocumentRoot() + "/" + req->path();

        if (!std::filesystem::exists(resource) || req->path() == "/") {
            auto const indexPath = fmt::format("{}/index.html", drogon::app().getDocumentRoot());

            acb(drogon::HttpResponse::newFileResponse(indexPath));
            return;
        }

        acb(drogon::HttpResponse::newFileResponse(resource));
    };

    auto& drogon_app = drogon::app()
                           .setDocumentRoot("./web/")
                           .registerPreRoutingAdvice(serveFrontendAdvice)
                           .enableCompressedRequest()
                           .addListener("0.0.0.0", 8080)
                           .setUploadPath("/tmp/bxt/")
                           .setClientMaxBodySize(256 * 1024 * 1024)
                           .setClientMaxMemoryBodySize(1024 * 1024);

    setup_scheduler(drogon_app, container.service<bxt::di::Utilities::IOScheduler>(),
                    container.service<bxt::di::Utilities::EventBus>());
    setup_controllers(drogon_app, container);

    drogon_app.run();

    return 0;
}
