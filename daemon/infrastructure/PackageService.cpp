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
#include "core/application/events/CommitEvent.h"
#include "core/application/events/IntegrationEventBase.h"
#include "core/application/RequestContext.h"
#include "core/domain/entities/Section.h"
#include "coro/task.hpp"
#include "coro/when_all.hpp"
#include "utilities/Error.h"
#include "utilities/StaticDTOMapper.h"

#include <algorithm>
#include <iterator>
#include <ranges>
#include <vector>

namespace bxt::Infrastructure {

coro::task<PackageService::Result<void>>
    PackageService::commit_transaction(PackageService::Transaction const transaction) {
    std::vector<PackageDTO> packages_to_add;

    packages_to_add.reserve(transaction.to_add.size());

    auto ids_to_delete =
        transaction.to_delete
        | std::views::transform([](PackageService::Transaction::PackageAction const& action) {
              return Package::TId {SectionDTOMapper::to_entity(action.section), Name(action.name)};
          })
        | std::ranges::to<std::vector>();

    auto uow = co_await m_uow_factory(true);

    std::vector<coro::task<PackageService::Result<void>>> tasks;
    for (auto const& package : transaction.to_add) {
        tasks.push_back(add_package(package, uow));
    }

    auto added = co_await coro::when_all(std::move(tasks));
    for (auto& added_package : added) {
        if (!added_package.return_value().has_value()) {
            co_return bxt::make_error_with_source<CrudError>(
                std::move(added_package.return_value().error()),
                CrudError::ErrorType::InternalError);
        }
    }

    auto deleted = co_await m_repository.delete_async(ids_to_delete, uow);
    if (!deleted.has_value()) {
        co_return bxt::make_error_with_source<CrudError>(std::move(deleted.error()),
                                                         CrudError::ErrorType::InternalError);
    }

    for (auto const& action : transaction.to_move) {
        auto move_result =
            co_await move_package(action.name, action.from_section, action.to_section, uow);
        if (!move_result.has_value()) {
            co_return bxt::make_error_with_source<CrudError>(std::move(move_result.error()),
                                                             CrudError::ErrorType::InternalError);
        }
    }

    for (auto const& action : transaction.to_copy) {
        auto copy_result =
            co_await copy_package(action.name, action.from_section, action.to_section, uow);
        if (!copy_result.has_value()) {
            co_return bxt::make_error_with_source<CrudError>(std::move(copy_result.error()),
                                                             CrudError::ErrorType::InternalError);
        }
    }

    auto commit_ok = co_await uow->commit_async();
    if (!commit_ok) {
        co_return bxt::make_error_with_source<CrudError>(std::move(commit_ok.error()),
                                                         CrudError::ErrorType::InternalError);
    }

    co_return {};
}
coro::task<PackageService::Result<void>>
    PackageService::move_package(std::string const package_name,
                                 PackageSectionDTO const from_section,
                                 PackageSectionDTO const to_section,
                                 std::shared_ptr<UnitOfWorkBase> unitofwork) {
    // First, find the package in the from_section
    auto package_to_move = co_await m_repository.find_by_section_async(
        SectionDTOMapper::to_entity(from_section), package_name, unitofwork);

    if (!package_to_move.has_value()) {
        co_return bxt::make_error_with_source<CrudError>(std::move(package_to_move.error()),
                                                         CrudError::ErrorType::EntityNotFound);
    }

    package_to_move->set_section(SectionDTOMapper::to_entity(to_section));

    // Then, add the package to the to_section
    auto saved = co_await m_repository.save_async(*package_to_move, unitofwork);

    if (!saved.has_value()) {
        co_return bxt::make_error_with_source<CrudError>(std::move(saved.error()),
                                                         CrudError::ErrorType::InternalError);
    }

    // Finally, remove the package from the from_section
    auto remove_result = co_await m_repository.delete_async(package_to_move->id(), unitofwork);

    if (!remove_result.has_value()) {
        co_return bxt::make_error_with_source<CrudError>(std::move(remove_result.error()),
                                                         CrudError::ErrorType::InternalError);
    }

    co_return {};
}

coro::task<PackageService::Result<void>>
    PackageService::copy_package(std::string const package_name,
                                 PackageSectionDTO const from_section,
                                 PackageSectionDTO const to_section,
                                 std::shared_ptr<UnitOfWorkBase> unitofwork) {
    // First, find the package in the from_section
    auto package_to_copy = co_await m_repository.find_by_section_async(
        SectionDTOMapper::to_entity(from_section), package_name, unitofwork);

    if (!package_to_copy.has_value()) {
        co_return bxt::make_error_with_source<CrudError>(std::move(package_to_copy.error()),
                                                         CrudError::ErrorType::EntityNotFound);
    }

    package_to_copy->set_section(SectionDTOMapper::to_entity(to_section));

    // Then, add the package to the to_section
    auto saved = co_await m_repository.save_async(*package_to_copy, unitofwork);

    if (!saved.has_value()) {
        co_return bxt::make_error_with_source<CrudError>(std::move(saved.error()),
                                                         CrudError::ErrorType::InternalError);
    }

    co_return {};
}

coro::task<PackageService::Result<void>>
    PackageService::add_package(PackageDTO const package,
                                std::shared_ptr<UnitOfWorkBase> unitofwork) {
    auto deployed_entity = PackageDTOMapper::to_entity(package);

    auto current_entitites = co_await m_repository.find_by_section_async(
        SectionDTOMapper::to_entity(package.section), unitofwork);

    if (!current_entitites.has_value()) {
        co_return bxt::make_error<CrudError>(CrudError::ErrorType::InvalidArgument);
    }

    auto current_entity = std::ranges::find(*current_entitites, package.name, &Package::name);

    if (current_entity != current_entitites->end()
        && deployed_entity.version() <= current_entity->version()) {
        co_return bxt::make_error<CrudError>(CrudError::ErrorType::EntityAlreadyExists);
    }

    auto saved = co_await m_repository.save_async(PackageDTOMapper::to_entity(package), unitofwork);

    if (!saved.has_value()) {
        co_return bxt::make_error_with_source<CrudError>(std::move(saved.error()),
                                                         CrudError::ErrorType::InternalError);
    }

    co_return {};
}

coro::task<PackageService::Result<std::vector<PackageDTO>>>
    PackageService::get_packages(PackageSectionDTO const section_dto) const {
    auto section = SectionDTOMapper::to_entity(section_dto);
    std::vector<PackageDTO> result;

    auto result_entities =
        co_await m_repository.find_by_section_async(section, co_await m_uow_factory());

    if (!result_entities.has_value()) {
        co_return result;
    }

    result.reserve(result_entities->size());
    std::ranges::transform(*result_entities, std::back_inserter(result), PackageDTOMapper::to_dto);

    co_return result;
}

coro::task<PackageService::Result<void>> PackageService::snap(PackageSectionDTO const from_section,
                                                              PackageSectionDTO const to_section) {
    auto uow = co_await m_uow_factory(true);

    auto source_packages =
        co_await m_repository.find_by_section_async(SectionDTOMapper::to_entity(from_section), uow);

    if (!source_packages.has_value()) {
        co_return bxt::make_error_with_source<CrudError>(std::move(source_packages.error()),
                                                         CrudError::ErrorType::EntityNotFound);
    }

    auto target_packages =
        co_await m_repository.find_by_section_async(SectionDTOMapper::to_entity(to_section), uow);

    if (!target_packages.has_value()) {
        co_return bxt::make_error_with_source<CrudError>(std::move(source_packages.error()),
                                                         CrudError::ErrorType::InternalError);
    }

    std::vector<Package::TId> target_ids;
    target_ids.reserve(target_packages->size());

    std::ranges::transform(*target_packages, std::back_inserter(target_ids), &Package::id);

    auto deleted = co_await m_repository.delete_async(target_ids, uow);

    if (!deleted.has_value()) {
        co_return bxt::make_error_with_source<CrudError>(std::move(deleted.error()),
                                                         CrudError::ErrorType::InternalError);
    }

    for (auto& source_package : *source_packages) {
        source_package.set_section(SectionDTOMapper::to_entity(to_section));
    }

    auto saved = co_await m_repository.save_async(*source_packages, uow);
    if (!saved.has_value()) {
        co_return bxt::make_error_with_source<CrudError>(std::move(saved.error()),
                                                         CrudError::ErrorType::InternalError);
    }

    auto commit_ok = co_await uow->commit_async();

    if (!commit_ok.has_value()) {
        co_return bxt::make_error_with_source<CrudError>(std::move(commit_ok.error()),
                                                         CrudError::ErrorType::InternalError);
    }

    co_return {};
}

coro::task<PackageService::Result<void>> PackageService::push(Transaction const transaction,
                                                              RequestContext const context) {
    auto packages_to_add = Utilities::map_entries(transaction.to_add, PackageDTOMapper::to_entity);

    auto ids_to_remove =
        transaction.to_delete | std::views::transform([](Transaction::PackageAction const& value) {
            return Package::TId {SectionDTOMapper::to_entity(value.section), Name(value.name)};
        })
        | std::ranges::to<std::vector>();

    auto result = co_await commit_transaction(transaction);

    if (!result.has_value()) {
        co_return bxt::make_error_with_source<CrudError>(std::move(result.error()),
                                                         CrudError::ErrorType::InternalError);
    }

    auto to_copy = transaction.to_copy;
    auto to_move = transaction.to_move;

    co_await m_dispatcher.dispatch_single_async<Core::Application::Events::IntegrationEventPtr>(
        std::make_shared<Core::Application::Events::Commited>(
            context.user_name, std::move(packages_to_add), std::move(ids_to_remove),
            std::move(to_move), std::move(to_copy)));
    co_return {};
}

coro::task<PackageService::Result<void>> PackageService::snap_branch(std::string const from_branch,
                                                                     std::string const to_branch,
                                                                     std::string const arch) {
    auto uow = co_await m_uow_factory(true);

    auto from_sections = co_await m_section_repository.find_async(
        [from_branch, arch](auto const& section) {
            return section.architecture() == arch && section.branch() == from_branch;
        },
        uow);

    if (!from_sections.has_value()) {
        co_return bxt::make_error_with_source<CrudError>(std::move(from_sections.error()),
                                                         CrudError::ErrorType::InternalError);
    }

    auto to_sections = co_await m_section_repository.find_async(
        [to_branch, arch](auto const& section) {
            return section.architecture() == arch && section.branch() == to_branch;
        },
        uow);

    for (auto const& section : *to_sections) {
        auto to_delete_packages = co_await m_repository.find_by_section_async(section, uow);
        if (!to_delete_packages.has_value()) {
            co_return bxt::make_error_with_source<CrudError>(std::move(to_delete_packages.error()),
                                                             CrudError::ErrorType::InternalError);
        }
        auto to_delete_ids = *to_delete_packages | std::views::transform([](auto const& package) {
            return package.id();
        }) | std::ranges::to<std::vector>();

        auto deleted = co_await m_repository.delete_async(to_delete_ids, uow);
    }

    std::vector<Package> packages;
    for (auto const& section : *from_sections) {
        auto from_packages = co_await m_repository.find_by_section_async(section, uow);

        if (!from_packages.has_value()) {
            co_return bxt::make_error_with_source<CrudError>(std::move(from_packages.error()),
                                                             CrudError::ErrorType::InternalError);
        }

        packages = *from_packages | std::views::transform([&](auto&& package) {
            auto package_section = package.section();
            package_section.set_branch(to_branch);
            package.set_section(package_section);
            return package;
        }) | std::ranges::to<std::vector>();
    }

    auto saved = co_await m_repository.save_async(packages, uow);

    co_await uow->commit_async();

    co_return {};
}
} // namespace bxt::Infrastructure
