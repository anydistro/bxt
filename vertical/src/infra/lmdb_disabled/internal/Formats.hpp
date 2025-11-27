/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2025 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
#pragma once

#include <rfl.hpp>
#include <rfl/Result.hpp>

#if BXT_LMDB_DATA_FORMAT == msgpack
    #include <rfl/msgpack.hpp>
#elif BXT_LMDB_DATA_FORMAT == bincode
    #include <rfl/bincode.hpp>
#elif BXT_LMDB_DATA_FORMAT == json
    #include <rfl/json.hpp>
#elif BXT_LMDB_DATA_FORMAT == avro
    #include <rfl/avro.hpp>
#elif BXT_LMDB_DATA_FORMAT == bson
    #include <rfl/bson.hpp>
#elif BXT_LMDB_DATA_FORMAT == capnp
    #include <rfl/capnp.hpp>
#elif BXT_LMDB_DATA_FORMAT == cbor
    #include <rfl/cbor.hpp>
#elif BXT_LMDB_DATA_FORMAT == flexbuffers
    #include <rfl/flexbuffers.hpp>
#elif BXT_LMDB_DATA_FORMAT == toml
    #include <rfl/toml.hpp>
#elif BXT_LMDB_DATA_FORMAT == ubjson
    #include <rfl/ubjson.hpp>
#elif BXT_LMDB_DATA_FORMAT == xml
    #include <rfl/xml.hpp>
#elif BXT_LMDB_DATA_FORMAT == yaml
    #include <rfl/yaml.hpp>
#elif BXT_LMDB_DATA_FORMAT == none
    #error "BXT_LMDB_DATA_FORMAT is not set"
#endif

namespace bxt::adapters::lmdb::internal {

template<typename T> auto serialize(T const& obj) -> std::vector<char> {
    return rfl::BXT_LMDB_DATA_FORMAT::write(obj);
}

template<typename T, typename... TArgs> auto deserialize(TArgs&&... args) -> rfl::Result<T> {
    return rfl::BXT_LMDB_DATA_FORMAT::read<T>(std::forward<TArgs>(args)...);
}

} // namespace bxt::adapters::lmdb::internal
