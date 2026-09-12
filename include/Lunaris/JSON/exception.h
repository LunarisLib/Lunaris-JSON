#pragma once

#include <stdexcept>

namespace Lunaris {
namespace JSON {

    class JsonException : public std::runtime_error {
    public:
        explicit JsonException(const std::string&) noexcept;
        explicit JsonException(const char*) noexcept;

        const char* what() const noexcept;
    };

} // namespace JSON
} // namespace Lunaris