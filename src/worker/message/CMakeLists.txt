set(WORKER_MESSAGE_SOURCES
    messages.cpp
)

set(WORKER_MESSAGE_HEADERS
    messages.hpp
)

add_library(worker_message
    ${WORKER_MESSAGE_SOURCES}
    ${WORKER_MESSAGE_HEADERS}
)

target_link_libraries(worker_message PRIVATE common)

if(Boost_FOUND)
    target_include_directories(worker_message PRIVATE ${Boost_INCLUDE_DIRS}) 
    target_link_libraries(worker_message PRIVATE ${Boost_LIBRARIES})
endif()