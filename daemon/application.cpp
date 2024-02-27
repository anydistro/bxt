/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2022 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#include "core/application/dtos/PackageSectionDTO.h"
#include "core/application/services/CompareService.h"
#include "core/application/services/DeploymentService.h"
#include "core/application/services/SectionService.h"
#include "core/domain/entities/User.h"
#include "core/domain/value_objects/Name.h"
#include "coro/io_scheduler.hpp"
#include "coro/sync_wait.hpp"
#include "di.h"
#include "drogon/HttpResponse.h"
#include "drogon/drogon_callbacks.h"
#include "drogon/utils/FunctionTraits.h"
#include "events.h"
#include "presentation/web-controllers/SectionController.h"
#include "utilities/Error.h"
#include "utilities/errors/DatabaseError.h"

#include <boost/log/trivial.hpp>
#include <boost/log/utility/setup.hpp>
#include <cstdlib>
#include <drogon/HttpAppFramework.h>
#include <filesystem>
#include <kangaru/debug.hpp>
#include <memory>
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

void setup_di_container(kgr::container& ctr) {
    using namespace bxt;

    ctr.emplace<di::Utilities::IOScheduler>(coro::io_scheduler::options {
        .thread_strategy = coro::io_scheduler::thread_strategy_t::manual,
    });

    ctr.service<di::Utilities::EventBus>();

    ctr.service<di::Infrastructure::EventLogger>();

    ctr.service<di::Utilities::EventBusDispatcher>();

    ctr.invoke<di::Utilities::RepoSchema::Parser,
               di::Infrastructure::ArchRepoOptions>(
        [](bxt::Utilities::RepoSchema::Parser& parser,
           bxt::Infrastructure::ArchRepoOptions& options) {
            parser.extend(&options);

            parser.parse("./box.yml");
        });

    ctr.invoke<di::Utilities::LMDB::Environment>(
        [](std::shared_ptr<bxt::Utilities::LMDB::Environment> lmdbenv) {
            lmdbenv->env().set_mapsize(1UL * 1024UL * 1024UL * 1024UL);
            lmdbenv->env().set_max_dbs(10);

            std::filesystem::create_directories("./bxtd.mdb/");

            lmdbenv->env().open("./bxtd.mdb/", 0, 0664);
        });

    ctr.service<di::Persistence::SectionRepository>();
    ctr.emplace<di::Persistence::UserRepository>(std::string("bxt::Users"));
    ctr.emplace<di::Persistence::PackageLogEntryRepository>(
        std::string("bxt::PackageLogs"));

    ctr.emplace<di::Persistence::Box::Pool>("./box/pool/");
    ctr.emplace<di::Persistence::Box::LMDBPackageStore>("./box/", "bxt::Box");

    ctr.emplace<di::Persistence::Box::AlpmDBExporter>("./box/");

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
        .registerController(ctr.service<PermissionController>())
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
        User defaultUser(Name("default"), "ILoveMacarons");

        coro::sync_wait(repository.add_async(defaultUser));
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
