#pragma once

#include "common/crack_hash_request/CrackHashRequest.hpp"

namespace ch {
namespace common {

class FullCrackHashRequest final : public CrackHashRequest {

public:

    static std::shared_ptr<CrackHashRequest> createFullCrackHashRequest(
        const std::string &hash,
        const std::size_t  maxLength,
        const std::string &alphabet);

    const std::string &hash()      const override { return m_hash; }
          std::size_t  maxLength() const override { return m_maxLength; }
    const std::string &alphabet()  const override { return m_alphabet; }

    std::vector<std::shared_ptr<CrackHashRequest>> split(std::size_t n) const override;

private:

    FullCrackHashRequest(
        const std::string &hash,
        const std::size_t  maxLength,
        const std::string &alphabet,
        const std::string &from,
        const std::string &to);

    const std::string m_hash;
    const std::size_t m_maxLength;
    const std::string m_alphabet;
};

} // namespace common
} // namespace ch