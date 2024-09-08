/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2022 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
#pragma once

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>

namespace bxt::Core::Domain {
namespace {
    namespace uuids = boost::uuids;
}
template<typename TId = uuids::uuid> class AggregateRoot {
public:
    virtual ~AggregateRoot() = default;

    TId const& id() const {
        return m_id;
    }

private:
    TId m_id;
};

} // namespace bxt::Core::Domain
