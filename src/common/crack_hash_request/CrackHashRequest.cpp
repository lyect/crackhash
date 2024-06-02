#include "CrackHashRequest.hpp"

#include "common/crack_hash_request/CrackHashRequestIterator.hpp"
#include "common/crack_hash_request/utilities.hpp"

#include <sstream>

namespace ch {
namespace common {

CrackHashRequest::CrackHashRequest(
        CrackHashRequestType type,
        const std::string &from,
        const std::string &to)
    : m_type{type}
    , m_from{from}
    , m_to{to}
{}

CrackHashRequestIterator CrackHashRequest::begin() const {
    return CrackHashRequestIterator(shared_from_this(), m_from);
}

CrackHashRequestIterator CrackHashRequest::end() const {
    return CrackHashRequestIterator(shared_from_this(), m_to);
}

std::string CrackHashRequest::toString() const {
    std::stringstream ss;
    
    ss << crackHashRequestTypeToString(type())
       <<  "[H=" << hash()
       << ", L=" << maxLength()
       << ", A=" << alphabet()
       << ", F=" << from()
       << ", T=" << to()
       << "]";
       
    return ss.str();
}

} // namespace common
} // namespace ch