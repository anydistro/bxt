/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2022 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
#pragma once

#include "AggregateRoot.h"
#include "core/domain/value_objects/Name.h"
#include "utilities/to_string.h"

#include <fmt/format.h>

namespace bxt::Core::Domain {

class Section {
public:
    Section(Name const& branch, Name const& repository, Name const& architecture)
        : m_branch(branch)
        , m_repository(repository)
        , m_architecture(architecture) {
    }

    using TId = std::string;

    TId id() const {
        return this->string();
    }

    Name branch() const {
        return m_branch;
    }
    void set_branch(Name const& new_branch) {
        m_branch = new_branch;
    }

    Name repository() const {
        return m_repository;
    }
    void set_repository(Name const& new_repository) {
        m_repository = new_repository;
    }

    Name architecture() const {
        return m_architecture;
    }
    void set_architecture(Name const& new_architecture) {
        m_architecture = new_architecture;
    }

    std::string string() const {
        return fmt::format("{}/{}/{}", branch(), repository(), architecture());
    }
    auto operator<=>(Section const& other) const = default;

private:
    Name m_branch;
    Name m_repository;
    Name m_architecture;
};

} // namespace bxt::Core::Domain

template<> inline std::string bxt::to_string(Core::Domain::Section const& section) {
    return section.string();
}
