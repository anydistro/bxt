/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2024 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */

#pragma once
#include <openssl/evp.h>
#include <string>
#include <vector>

namespace bxt::Utilities {

// For buffer size calculation details see https://docs.openssl.org/master/man3/EVP_EncodeInit

inline std::string b64_encode(std::string const& input) {
    size_t output_size = ((input.size() + 2) / 3) * 4 + 1;
    std::vector<unsigned char> output(output_size);
    int encoded_length =
        EVP_EncodeBlock(output.data(), reinterpret_cast<unsigned char const*>(input.data()),
                        static_cast<int>(input.size()));
    return {output.begin(), output.begin() + encoded_length};
}

inline std::string b64_decode(std::string_view input) {
    size_t output_size = 3 * input.size() / 4 + 1;
    std::vector<unsigned char> output(output_size);
    int decoded_length =
        EVP_DecodeBlock(output.data(), reinterpret_cast<unsigned char const*>(input.data()),
                        static_cast<int>(input.size()));
    if (decoded_length < 0) {
        throw std::runtime_error("Base64 decoding failed");
    }
    if (input.size() >= 2 && input[input.size() - 1] == '=') {
        decoded_length--;
        if (input[input.size() - 2] == '=') {
            decoded_length--;
        }
    }
    return {output.begin(), output.begin() + decoded_length};
}

} // namespace bxt::Utilities
