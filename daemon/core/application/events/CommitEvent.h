/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2024 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
#pragma once

#include "core/application/events/IntegrationEventBase.h"
#include "core/application/services/PackageService.h"
#include "core/domain/entities/Package.h"

#include <string>

namespace bxt::Core::Application::Events {

struct Commited : public IntegrationEventBase {
    Commited() = default;
    Commited(std::string user_name,
             std::vector<Domain::Package>&& to_add,
             std::vector<Domain::Package::TId>&& to_delete,
             std::vector<PackageService::Transaction::TransferAction>&& to_move,
             std::vector<PackageService::Transaction::TransferAction>&& to_copy)
        : user_name(std::move(user_name))
        , to_add(std::move(to_add))
        , to_delete(std::move(to_delete))
        , to_move(std::move(to_move))
        , to_copy(std::move(to_copy)) {
    }

    std::string user_name;

    std::vector<Domain::Package> to_add;
    std::vector<Domain::Package::TId> to_delete;
    std::vector<PackageService::Transaction::TransferAction> to_move;
    std::vector<PackageService::Transaction::TransferAction> to_copy;

    std::string message() const override {
        return fmt::format("User {} commited a transaction: {} packages added, {} "
                           "packages deleted",
                           user_name, to_add.size(), to_delete.size());
    }
};

} // namespace bxt::Core::Application::Events
