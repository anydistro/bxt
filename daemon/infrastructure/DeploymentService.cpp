/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2022 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: GPL-3.0-or-later
 *
 */
#include "DeploymentService.h"

#include "core/application/dtos/PackageSectionDTO.h"
#include "infrastructure/PackageService.h"
#include "utilities/Error.h"

#include <filesystem>
#include <fmt/format.h>
#include <iostream>
#include <random>
#include <stdexcept>
#include <vector>

namespace bxt::Infrastructure {

coro::task<DeploymentService::Result<uint64_t>>
    DeploymentService::deploy_start() {
    std::mt19937_64 engine(std::random_device {}());
    std::uniform_int_distribution<uint64_t> distribution;
    auto session_id = distribution(engine);

    m_session_packages.try_emplace(session_id, std::vector<PackageDTO>());

    co_return session_id;
}

coro::task<DeploymentService::Result<void>>
    DeploymentService::deploy_push(PackageDTO package, uint64_t session_id) {
    if (!std::filesystem::exists(
            package.pool_entries[Core::Domain::PoolLocation::Automated]
                .filepath)) {
        co_return bxt::make_error<Error>(Error::ErrorType::PackagePushFailed);
    }

    const auto sections = co_await m_section_repository.all_async();

    if (!sections.has_value()) {
        co_return bxt::make_error<Error>(Error::ErrorType::DeploymentFailed);
    }
    bool found = false;
    for (const auto section : *sections) {
        if (SectionDTOMapper::to_dto(section) == package.section) {
            found = true;
            break;
        }
    }

    if (!found) {
        co_return bxt::make_error<Error>(Error::ErrorType::InvalidArgument);
    }

    if (package.pool_entries[Core::Domain::PoolLocation::Automated]
            .signature_path) {
        if (!std::filesystem::exists(
                *package.pool_entries[Core::Domain::PoolLocation::Automated]
                     .signature_path)) {
            co_return bxt::make_error<Error>(
                Error::ErrorType::PackagePushFailed);
        }
    }

    m_session_packages.at(session_id).emplace_back(package);

    co_return {};
}

coro::task<DeploymentService::Result<void>>
    DeploymentService::verify_session(uint64_t session_id) {
    if (m_session_packages.count(session_id) > 0) { co_return {}; }

    co_return bxt::make_error<Error>(Error::ErrorType::InvalidSession);
}

coro::task<DeploymentService::Result<void>>
    DeploymentService::deploy_end(uint64_t session_id) {
    const auto &packages = m_session_packages.at(session_id);
    PackageService::Transaction transaction;
    transaction.to_add = packages;

    auto commit_result =
        co_await m_package_service.commit_transaction(transaction);

    if (!commit_result.has_value()) {
        co_return bxt::make_error_with_source<Error>(
            std::move(commit_result.error()),
            Error::ErrorType::DeploymentFailed);
    }

    m_session_packages.erase(session_id);

    co_return {};
}

} // namespace bxt::Infrastructure
