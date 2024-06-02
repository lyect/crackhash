#pragma once

#include "common/crack_hash_request/CrackHashRequest.hpp"

namespace ch {
namespace common {

class SplittedCrackHashRequest final : public CrackHashRequest {

    friend class FullCrackHashRequest;

public:

    const std::string &hash()      const override { return m_parentRequest->hash(); }
          std::size_t  maxLength() const override { return m_parentRequest->maxLength(); }
    const std::string &alphabet()  const override { return m_parentRequest->alphabet(); }

    std::vector<std::shared_ptr<CrackHashRequest>> split(std::size_t n) const override;

private:

    SplittedCrackHashRequest(
        std::shared_ptr<const CrackHashRequest> parentRequest,
        const std::string &from,
        const std::string &to);

    std::shared_ptr<const CrackHashRequest> m_parentRequest;
};

} // namespace common
} // namespace ch