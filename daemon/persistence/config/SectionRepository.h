/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2022 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
#pragma once

#include "core/domain/entities/Section.h"
#include "core/domain/repositories/RepositoryBase.h"
#include "core/domain/value_objects/Name.h"
#include "utilities/repo-schema/Parser.h"

#include <yaml-cpp/yaml.h>

namespace bxt::Persistence {

class SectionRepository
    : public Core::Domain::ReadOnlyRepositoryBase<bxt::Core::Domain::Section> {
    using Section = bxt::Core::Domain::Section;

public:
    SectionRepository(Utilities::RepoSchema::Parser &parser)
        : m_parser(parser) {}

    virtual coro::task<TResult> find_by_id_async(TId id) override;
    virtual coro::task<TResult>
        find_first_async(std::function<bool(const Section &)>) override;
    virtual coro::task<TResults>
        find_async(std::function<bool(const Section &)> condition) override;

    virtual coro::task<TResults> all_async() override;

private:
    Utilities::RepoSchema::Parser &m_parser;
};

} // namespace bxt::Persistence
