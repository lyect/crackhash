#pragma once

#include <iterator>
#include <memory>

namespace ch {
namespace common {

// Forward declaration
class CrackHashRequest;



class CrackHashRequestIterator final : public std::iterator<std::forward_iterator_tag, std::string> {

    friend class CrackHashRequest;

public:

    bool operator==(const CrackHashRequestIterator &other) const;
    bool operator!=(const CrackHashRequestIterator &other) const;

    CrackHashRequestIterator &operator++();

    const std::string &operator*() const {
        return m_value;
    }

    const std::string *operator->() const {
        return &m_value;
    }

private:

    CrackHashRequestIterator(
        std::shared_ptr<const CrackHashRequest> crackHashRequest,
        const std::string &value);

    std::shared_ptr<const CrackHashRequest> m_crackHashRequest;

    std::string m_value;
};

} // namespace common
} // namespace ch

