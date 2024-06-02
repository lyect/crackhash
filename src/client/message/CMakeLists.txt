set(CLIENT_MESSAGE_SOURCES
    messages.cpp
)

set(CLIENT_MESSAGE_HEADERS
    messages.hpp
)

add_library(client_message
    ${CLIENT_MESSAGE_SOURCES}
    ${CLIENT_MESSAGE_HEADERS}
)

if(Boost_FOUND)
    target_include_directories(client_message PRIVATE ${Boost_INCLUDE_DIRS}) 
    target_link_libraries(client_message PRIVATE ${Boost_LIBRARIES})
endif()