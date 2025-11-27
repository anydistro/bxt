/* === This file is part of percpptency ===
 *
 *   SPDX-FileCopyrightText: 2025 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
#pragma once

#include <memory>
#include <string>
#include <type_traits>

#include <cpperr/Error.hpp>
#include <cpperr/Factory.hpp>
#include <rfl/Reflector.hpp>
#include <rfl/Result.hpp>
#include <sqlgen/aggregations.hpp>
#include <sqlgen/create_table.hpp>
#include <sqlgen/delete_from.hpp>
#include <sqlgen/insert.hpp>
#include <sqlgen/read.hpp>
#include <sqlgen/select_from.hpp>
#include <sqlgen/to.hpp>
#include <sqlgen/where.hpp>
#include <sqlgen/write.hpp>

#include "../../core/AggregateRoot.hpp"
#include "../../core/Repository.hpp"
#include "SqlgenError.hpp"
#include "SqlgenUnitOfWork.hpp"

namespace percpptency::adapters::sqlgen {

using ::sqlgen::literals::operator""_c;

template<typename T>
concept Reflectable = requires { typename rfl::Reflector<T>; };

template<typename T> using ReflType_t = typename rfl::Reflector<T>::ReflType;

// Convert rfl::Error to CrudError
template<typename R>
[[nodiscard]] static auto map_crud_error(rfl::Error&& err, std::string_view ctx) {
    SqlgenError sqlgen_error(std::move(err));
    
    CrudError error_kind;
    switch (sqlgen_error.kind()) {
    case SqlgenError::Kind::TableNotFound:
    case SqlgenError::Kind::ColumnNotFound:
        error_kind = CrudError::NotFound;
        break;
    case SqlgenError::Kind::DuplicateKey:
        error_kind = CrudError::AlreadyExists;
        break;
    case SqlgenError::Kind::UniqueConstraintViolation:
    case SqlgenError::Kind::ForeignKeyViolation:
    case SqlgenError::Kind::NullConstraintViolation:
        error_kind = CrudError::ConstraintViolation;
        break;
    case SqlgenError::Kind::DataTypeMismatch:
    case SqlgenError::Kind::InvalidParameter:
    case SqlgenError::Kind::InvalidSyntax:
        error_kind = CrudError::InvalidData;
        break;
    case SqlgenError::Kind::TransactionError:
        error_kind = CrudError::ConcurrencyConflict;
        break;
    default:
        error_kind = CrudError::StorageError;
        break;
    }
    
    return std::unexpected(error_kind);
}

template<Reflectable TEntity>
class SqlgenRepository : public Repository<TEntity> {
    using SerializationType = ReflType_t<TEntity>;
    using Base = Repository<TEntity>;

public:
    explicit SqlgenRepository(std::shared_ptr<SqlgenUnitOfWork> uow)
        : m_unit_of_work(std::move(uow)) {
        auto create_res = get_transaction().and_then([&](auto const& trx) {
            return trx->connection().and_then(::sqlgen::create_table<SerializationType>);
        });
        if (!create_res) {
            throw std::runtime_error(create_res.error().what());
        }
    }

    typename Base::Result get_by_id(std::string id) override {
        return get_transaction()
            .and_then([id](auto const& trx) {
                return trx->connection().and_then(
                    ::sqlgen::read<SerializationType> 
                    | ::sqlgen::where("id"_c == id)
                    | ::sqlgen::limit(1));
            })
            .and_then([](SerializationType&& row) -> rfl::Result<TEntity> {
                return rfl::Reflector<TEntity>::to(row);
            })
            .transform_error([](auto&& err) {
                return map_crud_error<TEntity>(std::forward<decltype(err)>(err), 
                                               "get_by_id failed");
            });
    }

    typename Base::ResultVec all() override {
        return get_transaction()
            .and_then([](auto const& trx) {
                return trx->connection().and_then(
                    ::sqlgen::read<std::vector<SerializationType>>);
            })
            .and_then([](std::vector<SerializationType>&& rows) 
                      -> rfl::Result<std::vector<TEntity>> {
                std::vector<TEntity> out;
                out.reserve(rows.size());
                for (auto& r : rows) {
                    auto obj = rfl::Reflector<TEntity>::to(r);
                    if (obj) {
                        out.push_back(std::move(obj.value()));
                    }
                }
                return out;
            })
            .transform_error([](auto&& err) {
                return map_crud_error<std::vector<TEntity>>(
                    std::forward<decltype(err)>(err), "all failed");
            });
    }

    typename Base::ResultCount count() override {
        struct Cnt {
            std::size_t cnt = 0;
        };
        return get_transaction()
            .and_then([](auto const& trx) {
                return trx->connection().and_then(
                    ::sqlgen::select_from<SerializationType>(::sqlgen::count().as<"cnt">())
                    | ::sqlgen::to<std::vector<Cnt>>);
            })
            .and_then([](std::vector<Cnt>&& v) -> rfl::Result<std::size_t> {
                return v.empty() ? 0 : v.front().cnt;
            })
            .transform_error([](auto&& err) {
                return map_crud_error<std::size_t>(std::forward<decltype(err)>(err),
                                                    "count failed");
            });
    }

    typename Base::ResultVoid create(TEntity& entity) override {
        // Extract events first if this is an aggregate root
        std::vector<std::unique_ptr<DomainEvent>> events;
        if constexpr (std::is_base_of_v<AggregateRoot<TEntity>, TEntity>) {
            events = entity.take_uncommitted_events();
        }
        
        auto result = get_transaction()
            .and_then([&entity](auto const& trx) {
                auto ser = rfl::Reflector<TEntity>::from(entity);
                return trx->connection().and_then(::sqlgen::insert(std::move(ser)));
            })
            .transform([](auto&&) { })  // Convert to void
            .transform_error([](auto&& err) {
                return map_crud_error<TEntity>(std::forward<decltype(err)>(err),
                                               "create failed");
            });
        
        // Add events to UoW only if operation succeeded
        if (result && !events.empty()) {
            if (auto uow = m_unit_of_work.lock()) {
                uow->add_events(std::move(events));
            }
        }
        
        return result;
    }

    typename Base::ResultVoid update(TEntity& entity) override {
        // Extract events first if this is an aggregate root
        std::vector<std::unique_ptr<DomainEvent>> events;
        if constexpr (std::is_base_of_v<AggregateRoot<TEntity>, TEntity>) {
            events = entity.take_uncommitted_events();
        }
        
        auto result = get_transaction()
            .and_then([&entity](auto const& trx) {
                auto ser = rfl::Reflector<TEntity>::from(entity);
                return trx->connection().and_then(::sqlgen::write(std::move(ser)));
            })
            .transform([](auto&&) { })  // Convert to void
            .transform_error([](auto&& err) {
                return map_crud_error<TEntity>(std::forward<decltype(err)>(err),
                                               "update failed");
            });
        
        // Add events to UoW only if operation succeeded
        if (result && !events.empty()) {
            if (auto uow = m_unit_of_work.lock()) {
                uow->add_events(std::move(events));
            }
        }
        
        return result;
    }

    typename Base::ResultBool delete_by_id(std::string id) override {
        return get_transaction()
            .and_then([id](auto const& trx) {
                return trx->connection().and_then(
                    ::sqlgen::delete_from<SerializationType>
                    | ::sqlgen::where("id"_c == id));
            })
            .and_then([](auto&&) -> rfl::Result<bool> { return true; })
            .transform_error([](auto&& err) {
                return map_crud_error<bool>(std::forward<decltype(err)>(err),
                                            "delete failed");
            });
    }

private:
    [[nodiscard]] rfl::Result<std::shared_ptr<SqlgenUnitOfWork>> get_transaction() const {
        if (m_unit_of_work.expired()) {
            return rfl::error("Unit of work expired");
        }
        return m_unit_of_work.lock();
    }

    std::weak_ptr<SqlgenUnitOfWork> m_unit_of_work;
};

} // namespace percpptency::adapters::sqlgen
