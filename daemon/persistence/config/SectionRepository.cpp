/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2022 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
#include "SectionRepository.h"

#include "core/domain/repositories/ReadOnlyRepositoryBase.h"
#include "core/domain/value_objects/Name.h"
#include "utilities/Error.h"
#include "utilities/to_string.h"

namespace bxt::Persistence {
coro::task<SectionRepository::TResult>
    bxt::Persistence::SectionRepository::find_by_id_async(
        TId id, std::shared_ptr<UnitOfWorkBase> uow) {
    const auto& sections = m_parser.sections();
    auto it = std::find_if(
        sections.begin(), sections.end(),
        [&id](const auto& section) { return bxt::to_string(section) == id; });

    if (it != sections.end()) {
        co_return Section(it->branch, it->repository, it->architecture);
    }

    co_return bxt::make_error<ReadError>(ReadError::EntityNotFound);
}

coro::task<SectionRepository::TResult>
    bxt::Persistence::SectionRepository::find_first_async(
        std::function<bool(const Section&)> condition,
        std::shared_ptr<UnitOfWorkBase> uow) {
    const auto& sections = m_parser.sections();

    for (const auto& section : sections) {
        Section s(section.branch, section.repository, section.architecture);
        if (condition(s)) { co_return s; }
    }

    co_return bxt::make_error<ReadError>(ReadError::EntityNotFound);
}

coro::task<SectionRepository::TResults>
    bxt::Persistence::SectionRepository::find_async(
        std::function<bool(const Section&)> condition,
        std::shared_ptr<UnitOfWorkBase> uow) {
    const auto& sections = m_parser.sections();
    std::vector<Section> result;

    for (const auto& section : sections) {
        Section s(section.branch, section.repository, section.architecture);
        if (condition(s)) { result.push_back(std::move(s)); }
    }

    co_return result;
}

coro::task<SectionRepository::TResults>
    SectionRepository::all_async(std::shared_ptr<UnitOfWorkBase> uow) {
    const auto& sections = m_parser.sections();

    std::vector<Section> result;
    result.reserve(sections.size());

    std::ranges::transform(
        sections, std::back_inserter(result), [](const auto& section) {
            return Section(section.branch, section.repository,
                           section.architecture);
        });

    co_return result;
}

} // namespace bxt::Persistence
