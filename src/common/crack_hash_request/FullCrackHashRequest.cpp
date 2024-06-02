#include "FullCrackHashRequest.hpp"

#include "common/crack_hash_request/SplittedCrackHashRequest.hpp"
#include "common/crack_hash_request/number_system_utilities.hpp"

namespace ch {
namespace common {

std::shared_ptr<CrackHashRequest> FullCrackHashRequest::createFullCrackHashRequest(
    const std::string &hash,
    const std::size_t  maxLength,
    const std::string &alphabet
) {
    auto *fullCrackHashRequestRawPtr = \
        new FullCrackHashRequest(
            hash,
            maxLength,
            alphabet,
            (alphabet.empty() ? std::string() : std::string(1, alphabet.front())),
            (alphabet.empty() ? std::string() : std::string(maxLength, alphabet.back())));
    
    return std::shared_ptr<CrackHashRequest>(fullCrackHashRequestRawPtr);
}

std::vector<std::shared_ptr<CrackHashRequest>> FullCrackHashRequest::split(std::size_t n) const {
    std::vector<std::shared_ptr<CrackHashRequest>> result;
    result.reserve(n);

    if (m_alphabet.empty()) {
        for (std::size_t i = 0; i < n; ++i) {
            auto *splittedCrackHashRequestRawPtr = new SplittedCrackHashRequest(shared_from_this(), "", "");
            result.push_back(std::shared_ptr<CrackHashRequest>(splittedCrackHashRequestRawPtr));
        }
        return result;
    }

    std::uint64_t b1 = m_alphabet.size();
    std::uint64_t q = m_alphabet.size();
    std::uint64_t qn = 1;
    for (std::size_t i = 0; i < m_maxLength; ++i) {
        qn *= q;
    }

    std::uint64_t totalWorkload = b1 * (qn - 1) / (q - 1);

    std::uint64_t excessWorkload = totalWorkload % n;
    std::uint64_t singleWorkload = totalWorkload / n;

    std::uint64_t workloadDistributed = 0;

    for (std::size_t i = 0; i < n; ++i) {
        std::uint64_t currentWorkload = singleWorkload;
        if (excessWorkload != 0) {
            ++currentWorkload;
            --excessWorkload;
        }

        auto *splittedCrackHashRequestRawPtr = \
            new SplittedCrackHashRequest(
                shared_from_this(),
                std::move(dec2any(workloadDistributed, m_alphabet)),
                std::move(dec2any(workloadDistributed + currentWorkload - 1, m_alphabet)));
        result.push_back(std::shared_ptr<CrackHashRequest>(splittedCrackHashRequestRawPtr));

        workloadDistributed += currentWorkload;
    }

    return result;
}

FullCrackHashRequest::FullCrackHashRequest(
        const std::string &hash,
        const std::size_t  maxLength,
        const std::string &alphabet,
        const std::string &from,
        const std::string &to)
    : CrackHashRequest(CrackHashRequestType::FULL, from, to)
    , m_hash{hash}
    , m_maxLength{maxLength}
    , m_alphabet{alphabet}
{}

} // namespace common
} // namespace ch