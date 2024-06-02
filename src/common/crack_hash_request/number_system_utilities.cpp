#include "number_system_utilities.hpp"

#include <sstream>
#include <algorithm>

namespace ch {
namespace common {

std::string dec2any(std::uint64_t x, const std::string &alphabet) {
    std::stringstream ss;
    // Do one iteration manually in order to
    //  handle case when x=0
    ss << alphabet[x % alphabet.size()];
    x /= alphabet.size();
    while (x > 0) {
        x -= 1;
        ss << alphabet[x % alphabet.size()];
        x /= alphabet.size();
    }
    std::string s = std::move(ss.str());
    return {s.rbegin(), s.rend()};
}

void incAny(std::string &x, const std::string &alphabet) {
    std::uint8_t carry = 1;
    for (int i = x.size() - 1; i >= 0; --i) {
        std::size_t d = std::distance(
            alphabet.begin(),
            std::find(alphabet.begin(), alphabet.end(), x[i]));

        d += carry;
        if (d == alphabet.size()) {
            x[i] = alphabet[0];
        }
        else {
            x[i] = alphabet[d];
            carry = 0;
            break;
        }
    }

    if (carry == 1) {
        x = alphabet[0] + x;
    }
}

} // namespace common
} // namespace ch