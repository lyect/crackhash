#include "SubCrackHashRequest.hpp"

#include <stdexcept>

namespace ch {
namespace common {

std::shared_ptr<CrackHashRequest> SubCrackHashRequest::createSubCrackHashRequest(
    const std::string &hash,
    const std::size_t  maxLength,
    const std::string &alphabet,
    const std::string &from,
    const std::string &to
) {
    auto *subCrackHashRequestRawPtr = new SubCrackHashRequest( hash, maxLength, alphabet, from, to);
    return std::shared_ptr<CrackHashRequest>(subCrackHashRequestRawPtr);
}

std::vector<std::shared_ptr<CrackHashRequest>> SubCrackHashRequest::split(
    __attribute__((unused)) std::size_t n
) const {
    throw std::runtime_error("Not implemented");
}

SubCrackHashRequest::SubCrackHashRequest(
        const std::string &hash,
        const std::size_t  maxLength,
        const std::string &alphabet,
        const std::string &from,
        const std::string &to)
    : CrackHashRequest(CrackHashRequestType::FULL, from, to)
    , m_hash{hash}
    , m_maxLength{maxLength}
    , m_alphabet{alphabet}
{};

} // namespace common
} // namespace ch