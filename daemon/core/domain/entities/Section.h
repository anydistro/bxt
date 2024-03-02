/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2022 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
#pragma once

#include "AggregateRoot.h"
#include "core/domain/value_objects/Name.h"

#include <fmt/format.h>

namespace bxt::Core::Domain {

class Section : public AggregateRoot<> {
public:
    Section(const Name& branch,
            const Name& repository,
            const Name& architecture)
        : m_branch(branch),
          m_repository(repository),
          m_architecture(architecture) {}

    Name branch() const { return m_branch; }
    void set_branch(const Name& new_branch) { m_branch = new_branch; }

    Name repository() const { return m_repository; }
    void set_repository(const Name& new_repository) {
        m_repository = new_repository;
    }

    Name architecture() const { return m_architecture; }
    void set_architecture(const Name& new_architecture) {
        m_architecture = new_architecture;
    }

    std::string string() const {
        return fmt::format("{}/{}/{}", branch(), repository(), architecture());
    }

private:
    Name m_branch;
    Name m_repository;
    Name m_architecture;
};

} // namespace bxt::Core::Domain
