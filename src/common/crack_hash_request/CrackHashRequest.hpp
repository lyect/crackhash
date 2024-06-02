#pragma once

#include "common/crack_hash_request/CrackHashRequestType.hpp"

#include <string>
#include <memory>
#include <vector>

namespace ch {
namespace common {

// Forward declaration
class CrackHashRequestIterator;



class CrackHashRequest : public std::enable_shared_from_this<CrackHashRequest> {

public:

    CrackHashRequestType type() const { return m_type; }
    const std::string &from()   const { return m_from; }
    const std::string &to()     const { return m_to; }

    virtual const std::string &hash()      const = 0;
    virtual       std::size_t  maxLength() const = 0;
    virtual const std::string &alphabet()  const = 0;

    virtual std::vector<std::shared_ptr<CrackHashRequest>> split(std::size_t n) const = 0;

    CrackHashRequestIterator begin() const;
    CrackHashRequestIterator end() const;

    std::string toString() const;

    bool operator==(const CrackHashRequest &other) const {
        return hash() == other.hash() && \
               maxLength() == other.maxLength() && \
               alphabet() == other.alphabet() && \
               from() == other.from() && \
               to() == other.to();
    }

    bool operator!=(const CrackHashRequest &other) const {
        return !(*this == other);
    }

protected:

    CrackHashRequest(
        CrackHashRequestType type,
        const std::string &from,
        const std::string &to);

private:

    const CrackHashRequestType m_type;
    const std::string m_from;
    const std::string m_to;
};

} // namespace common
} // namespace ch

