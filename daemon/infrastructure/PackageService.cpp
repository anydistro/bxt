/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2023 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
#include "PackageService.h"

#include "core/application/dtos/PackageDTO.h"
#include "core/application/dtos/PackageSectionDTO.h"
#include "core/application/errors/CrudError.h"
#include "coro/task.hpp"
#include "coro/when_all.hpp"
#include "utilities/Error.h"

#include <algorithm>
#include <iterator>
#include <vector>

namespace bxt::Infrastructure {

coro::task<PackageService::Result<void>> PackageService::commit_transaction(
    const PackageService::Transaction transaction) {
    std::vector<PackageDTO> packages_to_add;

    packages_to_add.reserve(transaction.to_add.size());

    auto ids_to_remove =
        transaction.to_remove
        | std::views::transform(
            [](const std::pair<PackageSectionDTO, std::string> &value) {
                return Package::TId {SectionDTOMapper::to_entity(value.first),
                                     Name(value.second)};
            })
        | std::ranges::to<std::vector>();

    auto uow = co_await m_uow_factory(true);

    std::vector<coro::task<PackageService::Result<void>>> tasks;
    for (const auto &package : transaction.to_add) {
        tasks.push_back(add_package(package, uow));
    }

    auto added = co_await coro::when_all(std::move(tasks));
    for (auto &added_package : added) {
        if (!added_package.return_value().has_value()) {
            co_return bxt::make_error_with_source<CrudError>(
                std::move(added_package.return_value().error()),
                CrudError::ErrorType::InternalError);
        }
    }
    auto deleted = co_await m_repository.delete_async(ids_to_remove, uow);
    if (!deleted.has_value()) {
        co_return bxt::make_error_with_source<CrudError>(
            std::move(deleted.error()), CrudError::ErrorType::InternalError);
    }

    auto commit_ok = co_await uow->commit_async();
    if (!commit_ok) {
        co_return bxt::make_error_with_source<CrudError>(
            std::move(commit_ok.error()), CrudError::ErrorType::InternalError);
    }

    co_return {};
}

coro::task<PackageService::Result<void>>
    PackageService::add_package(const PackageDTO package,
                                std::shared_ptr<UnitOfWorkBase> unitofwork) {
    auto deployed_entity = PackageDTOMapper::to_entity(package);

    auto current_entitites = co_await m_repository.find_by_section_async(
        SectionDTOMapper::to_entity(package.section), unitofwork);

    if (!current_entitites.has_value()) {
        co_return bxt::make_error<CrudError>(
            CrudError::ErrorType::InvalidArgument);
    }

    auto current_entity =
        std::ranges::find(*current_entitites, package.name, &Package::name);

    if (current_entity != current_entitites->end()
        && deployed_entity.version() <= current_entity->version()) {
        co_return bxt::make_error<CrudError>(
            CrudError::ErrorType::EntityAlreadyExists);
    }

    auto add_ok = co_await m_repository.add_async(
        PackageDTOMapper::to_entity(package), unitofwork);

    if (!add_ok.has_value()) {
        co_return bxt::make_error_with_source<CrudError>(
            std::move(add_ok.error()), CrudError::ErrorType::InternalError);
    }

    co_return {};
}

coro::task<PackageService::Result<std::vector<PackageDTO>>>
    PackageService::get_packages(const PackageSectionDTO section_dto) const {
    auto section = SectionDTOMapper::to_entity(section_dto);
    std::vector<PackageDTO> result;

    auto result_entities = co_await m_repository.find_by_section_async(
        section, co_await m_uow_factory());

    if (!result_entities.has_value()) { co_return result; }

    result.reserve(result_entities->size());
    std::ranges::transform(*result_entities, std::back_inserter(result),
                           PackageDTOMapper::to_dto);

    co_return result;
}

coro::task<PackageService::Result<void>>
    PackageService::snap(const PackageSectionDTO from_section,
                         const PackageSectionDTO to_section) {
    auto uow = co_await m_uow_factory(true);

    auto source_packages = co_await m_repository.find_by_section_async(
        SectionDTOMapper::to_entity(from_section), uow);

    if (!source_packages.has_value()) {
        co_return bxt::make_error_with_source<CrudError>(
            std::move(source_packages.error()),
            CrudError::ErrorType::EntityNotFound);
    }

    auto target_packages = co_await m_repository.find_by_section_async(
        SectionDTOMapper::to_entity(to_section), uow);

    if (!target_packages.has_value()) {
        co_return bxt::make_error_with_source<CrudError>(
            std::move(source_packages.error()),
            CrudError::ErrorType::InternalError);
    }

    std::vector<Package::TId> target_ids;
    target_ids.reserve(target_packages->size());

    std::ranges::transform(*target_packages, std::back_inserter(target_ids),
                           &Package::id);

    auto deleted = co_await m_repository.delete_async(target_ids, uow);

    if (!deleted.has_value()) {
        co_return bxt::make_error_with_source<CrudError>(
            std::move(deleted.error()), CrudError::ErrorType::InternalError);
    }

    for (auto &source_package : *source_packages) {
        source_package.set_section(SectionDTOMapper::to_entity(to_section));
    }

    auto added = co_await m_repository.add_async(*source_packages, uow);
    if (!added.has_value()) {
        co_return bxt::make_error_with_source<CrudError>(
            std::move(added.error()), CrudError::ErrorType::InternalError);
    }

    auto commit_ok = co_await uow->commit_async();

    if (!commit_ok.has_value()) {
        co_return bxt::make_error_with_source<CrudError>(
            std::move(commit_ok.error()), CrudError::ErrorType::InternalError);
    }

    co_return {};
}
} // namespace bxt::Infrastructure
