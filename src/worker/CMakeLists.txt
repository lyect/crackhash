set(WORKER_SOURCES
    config/WorkerConfigParser.cpp
    
    Worker.cpp

    main.cpp
)

set(WORKER_HEADERS
    config/WorkerConfig.hpp
    config/WorkerConfigParser.hpp

    Worker.hpp
)

add_executable(worker
    ${WORKER_SOURCES}
    ${WORKER_HEADERS}
)

target_link_libraries(worker PRIVATE common amqpcpp manager_message cryptopp)

add_subdirectory(message)

if(Boost_FOUND)
    target_include_directories(manager PRIVATE ${Boost_INCLUDE_DIRS}) 
    target_link_libraries(manager PRIVATE ${Boost_LIBRARIES})
endif()