#include <string>

namespace ch {
namespace common {

std::string dec2any(std::uint64_t x, const std::string &alphabet);

void incAny(std::string &x, const std::string &alphabet);

} // namespace ch
} // namespace common