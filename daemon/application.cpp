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
#include "core/domain/value_objects/Name.h"
#include "core/domain/value_objects/Permission.h"
#include "coro/io_scheduler.hpp"
#include "coro/sync_wait.hpp"
#include "di.h"
#include "drogon/HttpResponse.h"
#include "drogon/drogon_callbacks.h"
#include "drogon/utils/FunctionTraits.h"
#include "events.h"
#include "presentation/JwtOptions.h"
#include "presentation/cli-controllers/DeploymentOptions.h"
#include "presentation/web-controllers/SectionController.h"
#include "utilities/Error.h"
#include "utilities/configuration/Configuration.h"
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
                    boost::log::keywords::format =
                        "[%TimeStamp%][%Severity%]: %Message%",
                    boost::log::keywords::auto_flush = true);
    add_file_log("journal.log",
                 boost::log::keywords::format =
                     "[%TimeStamp%][%Severity%]: %Message%",
                 boost::log::keywords::auto_flush = true);
}

toml::table setup_toml_configuration(const std::filesystem::path& config_path) {
    if (!std::filesystem::exists(config_path)) {
        std::ofstream stream {config_path};
    }

    auto result = toml::parse_file(config_path.string());

    if (!result) { return toml::table {}; }

    return result.table();
}

void setup_di_container(kgr::container& ctr) {
    using namespace bxt;

    ctr.emplace<di::Utilities::IOScheduler>(coro::io_scheduler::options {
        .thread_strategy = coro::io_scheduler::thread_strategy_t::manual,
    });

    ctr.service<di::Utilities::EventBus>();

    ctr.service<di::Infrastructure::EventLogger>();

    ctr.service<di::Utilities::EventBusDispatcher>();

    ctr.emplace<di::Utilities::Configuration>(
        setup_toml_configuration("config.toml"));

    ctr.invoke<di::Utilities::Configuration, di::Utilities::LMDB::LMDBOptions,
               di::Persistence::Box::BoxOptions, di::Presentation::JwtOptions,
               di::Presentation::DeploymentOptions>(
        [](bxt::Utilities::Configuration& configuration,
           bxt::Utilities::LMDB::LMDBOptions& lmdb_options,
           bxt::Persistence::Box::BoxOptions& box_options,
           bxt::Presentation::JwtOptions& jwt_options,
           bxt::Presentation::DeploymentOptions& deployment_options) {
            lmdb_options.deserialize(configuration);
            box_options.deserialize(configuration);
            jwt_options.deserialize(configuration);
            deployment_options.deserialize(configuration);
        });

    ctr.invoke<di::Utilities::RepoSchema::Parser,
               di::Infrastructure::ArchRepoOptions,
               di::Persistence::Box::PoolOptions>(
        [](bxt::Utilities::RepoSchema::Parser& parser,
           bxt::Infrastructure::ArchRepoOptions& options,
           bxt::Persistence::Box::PoolOptions& pool_options) {
            parser.extend(&options);
            parser.extend(&pool_options);

            parser.parse("./box.yml");
        });

    ctr.invoke<di::Utilities::LMDB::Environment,
               di::Utilities::LMDB::LMDBOptions>(
        [](std::shared_ptr<bxt::Utilities::LMDB::Environment> lmdbenv,
           Utilities::LMDB::LMDBOptions& options) {
            lmdbenv->env().set_mapsize(1UL * 1024UL * 1024UL * 1024UL);
            lmdbenv->env().set_max_dbs(10);

            std::error_code ec;
            if (std::filesystem::create_directories(options.lmdb_path, ec);
                ec.value()) {
                logf("Cannot create LMDB folder. The error is \"{}\". Exiting.",
                     ec.message());
                exit(1);
            }

            try {
                lmdbenv->env().open(options.lmdb_path.c_str(), 0, 0664);
            } catch (const lmdb::error& er) {
                logf("Cannot open LMDB database. The error is \"{}\". Exiting.",
                     er.what());
                exit(1);
            }
        });

    ctr.service<di::Persistence::SectionRepository>();
    ctr.emplace<di::Persistence::UserRepository>(std::string("bxt::Users"));
    ctr.emplace<di::Persistence::PackageLogEntryRepository>(
        std::string("bxt::PackageLogs"));

    ctr.emplace<di::Persistence::Box::Pool>();
    ctr.emplace<di::Persistence::Box::LMDBPackageStore>("bxt::Box");

    ctr.emplace<di::Persistence::Box::AlpmDBExporter>();

    ctr.service<di::Persistence::Box::BoxRepository>();

    ctr.service<di::Core::Application::AuthService>();
    ctr.service<di::Core::Application::PermissionService>();

    ctr.service<di::Infrastructure::PackageService>();
    ctr.service<di::Infrastructure::DeploymentService>();
    ctr.service<di::Infrastructure::WSController>();

    ctr.service<di::Infrastructure::ArchRepoSyncService>();
    ctr.service<di::Core::Application::CompareService>();
}

void setup_controllers(drogon::HttpAppFramework& app, kgr::container& ctr) {
    using namespace bxt::di::Presentation;

    app.registerController(ctr.service<DeploymentController>())
        .registerController(ctr.service<PackageController>())
        .registerController(ctr.service<CompareController>())
        .registerController(ctr.service<AuthController>())
        .registerController(ctr.service<UserController>())
        .registerController(ctr.service<LogController>())
        .registerController(ctr.service<SectionController>())
        .registerController(
            ctr.service<bxt::di::Infrastructure::WSController>())
        .registerFilter(ctr.service<JwtFilter>());
}

void setup_scheduler(auto& app, auto scheduler, auto eventbus) {
    app.getLoop()->runEvery(1.0, [scheduler, eventbus]() {
        scheduler->process_events();
        eventbus->process();
    });
}

void setup_defaults(kgr::container& ctr) {
    using namespace bxt;

    auto& repository = ctr.service<di::Core::Domain::UserRepository>();

    const auto users = coro::sync_wait(repository.all_async());

    if (!users.has_value()) {
        BOOST_LOG_TRIVIAL(error) << users.error().what();
        abort();
    }

    if (users->empty()) {
        User default_user(Name("default"), "ILoveMacarons");
        default_user.set_permissions({Permission("*")});

        coro::sync_wait(repository.add_async(default_user));
        coro::sync_wait(repository.commit_async());
    }
}

int main() {
    setup_logger();

    kgr::container ctr;

    setup_di_container(ctr);

    setup_defaults(ctr);

    const auto serveFrontendAdvice = [](const drogon::HttpRequestPtr& req,
                                        drogon::AdviceCallback&& acb,
                                        drogon::AdviceChainCallback&& accb) {
        if (req->path().starts_with("/api/")) {
            accb();
            return;
        }

        const auto resource =
            drogon::app().getDocumentRoot() + "/" + req->path();

        if (!std::filesystem::exists(resource) || req->path() == "/") {
            const auto indexPath =
                fmt::format("{}/index.html", drogon::app().getDocumentRoot());

            acb(drogon::HttpResponse::newFileResponse(indexPath));
            return;
        }

        acb(drogon::HttpResponse::newFileResponse(resource));
    };

    auto& app = drogon::app()
                    .setDocumentRoot("../frontend/")
                    .registerPreRoutingAdvice(serveFrontendAdvice)
                    .addListener("0.0.0.0", 8080)
                    .setUploadPath("/tmp/bxt/")
                    .setClientMaxBodySize(256 * 1024 * 1024)
                    .setClientMaxMemoryBodySize(1024 * 1024);

    setup_scheduler(app, ctr.service<bxt::di::Utilities::IOScheduler>(),
                    ctr.service<bxt::di::Utilities::EventBus>());
    setup_controllers(app, ctr);

    app.run();

    return 0;
}
