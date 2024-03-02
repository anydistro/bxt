/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2022 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
#include "SectionRepository.h"

#include "core/domain/value_objects/Name.h"

namespace bxt::Persistence {

coro::task<SectionRepository::TResult>
    bxt::Persistence::SectionRepository::find_by_id_async(TId id) {
}

coro::task<SectionRepository::TResult>
    bxt::Persistence::SectionRepository::find_first_async(
        std::function<bool(const Section&)>) {
}

coro::task<SectionRepository::TResults>
    bxt::Persistence::SectionRepository::find_async(
        std::function<bool(const Section&)> condition) {
}

coro::task<SectionRepository::TResults> SectionRepository::all_async() {
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
