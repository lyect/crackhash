set(MANAGER_MESSAGE_SOURCES
    messages.cpp
)

set(MANAGER_MESSAGE_HEADERS
    messages.hpp
)

add_library(manager_message
    ${MANAGER_MESSAGE_SOURCES}
    ${MANAGER_MESSAGE_HEADERS}
)

if(Boost_FOUND)
    target_include_directories(manager_message PRIVATE ${Boost_INCLUDE_DIRS}) 
    target_link_libraries(manager_message PRIVATE ${Boost_LIBRARIES})
endif()