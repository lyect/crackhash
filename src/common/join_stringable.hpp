#pragma once

#include <sstream>

namespace {
    
    void joinStringable_internal(__attribute__((unused)) std::stringstream &ss) {
        /* Nothing to do */
    } 

    template<typename T, typename... ArgTypes>
    void joinStringable_internal(std::stringstream &ss, T t, ArgTypes... args) {
        ss << t;
        joinStringable_internal(ss, args...);
    }
}

namespace ch {
namespace common {

template<typename... ArgTypes>
std::string joinStringable(ArgTypes... args) {
    std::stringstream ss;
    joinStringable_internal(ss, args...);
    return ss.str();
}

} // namespace common
} // namespace ch