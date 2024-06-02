set(COMMON_SOURCES
    broker/CallbackLibBoostAsioHandler.cpp

    config/CommonConfig.cpp
    config/utilities.cpp

    crack_hash_request/CrackHashRequest.cpp
    crack_hash_request/CrackHashRequestIterator.cpp
    crack_hash_request/FullCrackHashRequest.cpp
    crack_hash_request/number_system_utilities.cpp
    crack_hash_request/SplittedCrackHashRequest.cpp
    crack_hash_request/SubCrackHashRequest.cpp
    crack_hash_request/utilities.cpp

    run_delayed.cpp
)

set(COMMON_HEADERS
    broker/CallbackLibBoostAsioHandler.hpp

    config/CommonConfig.hpp
    config/FileLoggerConfig.hpp
    config/LoggerConfig.hpp
    config/MessageBrokerConfig.hpp
    config/StdLoggerConfig.hpp
    config/utilities.hpp

    crack_hash_request/CrackHashRequest.hpp
    crack_hash_request/CrackHashRequestIterator.hpp
    crack_hash_request/CrackHashRequestStatus.hpp
    crack_hash_request/CrackHashRequestType.hpp
    crack_hash_request/FullCrackHashRequest.hpp
    crack_hash_request/number_system_utilities.hpp
    crack_hash_request/SplittedCrackHashRequest.hpp
    crack_hash_request/SubCrackHashRequest.hpp
    crack_hash_request/utilities.hpp

    join_stringable.hpp
    run_delayed.hpp
)

add_library(common
    ${COMMON_SOURCES}
    ${COMMON_HEADERS}
)

if(Boost_FOUND)
    target_include_directories(common PRIVATE ${Boost_INCLUDE_DIRS}) 
    target_link_libraries(common PRIVATE ${Boost_LIBRARIES})
endif()