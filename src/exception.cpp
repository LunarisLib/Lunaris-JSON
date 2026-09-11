#include <Lunaris/JSON/exception.h>

namespace Lunaris {
namespace JSON {

    JsonException::JsonException(const std::string& msg) noexcept
        : std::runtime_error(msg)
    {
    }

    JsonException::JsonException(const char* msg) noexcept
        : std::runtime_error(msg)
    {
    }

    const char* JsonException::what() const noexcept {
        return std::runtime_error::what();
    }

} // namespace Json
} // namespace Lunaris