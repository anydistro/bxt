/* === cpperr - Modern C++ Error Handling ===
 *
 *   SPDX-FileCopyrightText: 2025 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: MIT
 *
 */
#pragma once

/**
 * cpperr - A modern, Rust-inspired error handling library for C++20+
 * 
 * Core features:
 * - Type-safe error kinds (enums)
 * - Automatic source location tracking
 * - Flattened error cause chains (no heap allocations)
 * - Rust-style ? operator via macros
 * - From/Into trait pattern via ErrorConverter
 * 
 * Basic usage:
 * 
 *   enum class MyError { NotFound, InvalidInput };
 *   
 *   // Provide to_string() in same namespace as error type
 *   std::string to_string(MyError e) {
 *       switch (e) {
 *           case MyError::NotFound: return "NotFound";
 *           case MyError::InvalidInput: return "InvalidInput";
 *       }
 *   }
 *   
 *   using namespace cpperr;
 *   
 *   result<User, MyError> find_user(int id) {
 *       if (!exists(id)) {
 *           return make_error(MyError::NotFound, "user not in database");
 *       }
 *       return User{id};
 *   }
 *   
 *   result<User, MyError> get_user(int id) {
 *       auto user = TRY(find_user(id));  // Propagates error or extracts value
 *       return user;
 *   }
 * 
 * Error conversion:
 * 
 *   enum class LowLevelError { SqlError, IoError };
 *   enum class HighLevelError { DatabaseError };
 *   
 *   // Define conversion function
 *   template<>
 *   HighLevelError convert_error<HighLevelError>(LowLevelError e) {
 *       return HighLevelError::DatabaseError;
 *   }
 *   
 *   result<Data, HighLevelError> get_data() {
 *       auto low_level = TRY_AUTO(db_query(), HighLevelError);  // Auto-converts
 *       return low_level;
 *   }
 */

#include "cpperr/Error.hpp"
#include "cpperr/Factory.hpp"
#include "cpperr/Macros.hpp"