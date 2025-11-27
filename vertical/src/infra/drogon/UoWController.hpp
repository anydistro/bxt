/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2025 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */

#pragma once

#include <boost/di.hpp>
#include <drogon/HttpController.h>

#include "infra/sqlgen/SqlgenUnitOfWorkFactory.hpp"

namespace bxt::adapters::drogon {

template<typename TController> class CrudController : public ::drogon::HttpController<TController> {
public:
    using TController::TController;

    BOOST_DI_INJECT(CrudController, sqlgen::SqlgenUnitOfWorkFactory uow_factory)
        : TController(uow_factory)
        , m_uow_factory(uow_factory) {
    }

private:
    sqlgen::SqlgenUnitOfWorkFactory m_uow_factory;
};
} // namespace bxt::adapters::drogon
