/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2023 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
#pragma once
#include "core/domain/entities/Package.h"
#include "core/domain/entities/User.h"

#include <boost/uuid/uuid.hpp>
#include <vector>

namespace bxt::Core::Domain::Transactions {
class Transaction {
    const boost::uuids::uuid &id() const { return m_id; }

    std::vector<bxt::Core::Domain::Package> packages() const {
        return m_packages;
    }

    bxt::Core::Domain::User user() const { return m_user; }

private:
    boost::uuids::uuid m_id;
    std::vector<bxt::Core::Domain::Package> m_packages;
    bxt::Core::Domain::User m_user;
};
} // namespace bxt::Core::Domain::Transactions
