#include "SplittedCrackHashRequest.hpp"

#include <stdexcept>

namespace ch {
namespace common {

std::vector<std::shared_ptr<CrackHashRequest>> SplittedCrackHashRequest::split(
    __attribute__((unused)) std::size_t n
) const {
    throw std::runtime_error("Not implemented");
}

SplittedCrackHashRequest::SplittedCrackHashRequest(
        std::shared_ptr<const CrackHashRequest> parentRequest,
        const std::string &from,
        const std::string &to)
    : CrackHashRequest(CrackHashRequestType::SPLITTED, from, to)
    , m_parentRequest{parentRequest}
{}

} // namespace common
} // namespace ch