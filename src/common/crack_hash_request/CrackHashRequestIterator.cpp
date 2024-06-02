#include "CrackHashRequestIterator.hpp"

#include "common/crack_hash_request/CrackHashRequest.hpp"
#include "common/crack_hash_request/number_system_utilities.hpp"

namespace ch {
namespace common {

CrackHashRequestIterator::CrackHashRequestIterator(
    std::shared_ptr<const CrackHashRequest> crackHashRequest,
    const std::string &value)
        : m_crackHashRequest{crackHashRequest}
        , m_value{value}
{}

bool CrackHashRequestIterator::operator==(const CrackHashRequestIterator &other) const {
    return *m_crackHashRequest == *other.m_crackHashRequest && m_value == other.m_value;
}

bool CrackHashRequestIterator::operator!=(const CrackHashRequestIterator &other) const {
    return !(*this == other);
}

CrackHashRequestIterator &CrackHashRequestIterator::operator++() {
    if (m_value.empty()) {
        return *this;
    }

    if (m_value == m_crackHashRequest->to()) {
        m_value.clear();
        return *this;
    }

    incAny(m_value, m_crackHashRequest->alphabet());
    return *this;
}

} // namespace common
} // namespace ch
