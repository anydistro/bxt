/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2023 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
#include "SectionService.h"

#include "core/application/dtos/PackageSectionDTO.h"
#include "core/application/errors/CrudError.h"
#include "utilities/Error.h"

#include <iterator>
#include <ranges>
namespace bxt::Core::Application {

coro::task<SectionService::Result<std::vector<PackageSectionDTO>>>
    SectionService::get_sections() const {
    std::vector<PackageSectionDTO> result;

    auto sections = co_await m_repository.all_async();

    if (!sections.has_value()) {
        co_return bxt::make_error_with_source<CrudError>(
            std::move(sections.error()), CrudError::ErrorType::EntityNotFound);
    }

    result.reserve(sections->size());

    std::ranges::transform(*sections, std::back_inserter(result),
                           SectionDTOMapper::to_dto);

    co_return result;
}

} // namespace bxt::Core::Application
