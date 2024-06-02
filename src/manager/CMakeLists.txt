set(MANAGER_SOURCES
    config/ManagerConfigParser.cpp
    
    db/CrackHashRequestsDbHandler.cpp

    http/IncomingHttpConnection.cpp
    http/IncomingHttpConnectionHandler.cpp
    http/utilities.cpp

    Manager.cpp

    main.cpp
)

set(MANAGER_HEADERS
    config/CrackHashRequestDatabaseConfig.hpp
    config/IncomingHttpConnectionConfig.hpp
    config/ManagerConfig.hpp
    config/ManagerConfigParser.hpp
    
    db/CrackHashRequestsDbHandler.hpp

    http/IncomingHttpConnection.hpp
    http/IncomingHttpConnectionHandler.hpp
    http/types.hpp
    http/utilities.hpp

    Manager.hpp
)

add_executable(manager
    ${MANAGER_SOURCES}
    ${MANAGER_HEADERS}
)

target_link_libraries(manager PRIVATE common amqpcpp client_message worker_message)

add_subdirectory(message)

if(Boost_FOUND)
    target_include_directories(manager PRIVATE ${Boost_INCLUDE_DIRS}) 
    target_link_libraries(manager PRIVATE ${Boost_LIBRARIES})
endif()

find_package(bsoncxx REQUIRED)
find_package(mongocxx REQUIRED)

target_include_directories(manager PRIVATE /usr/local/include/mongocxx/v_noabi/mongocxx/)
target_include_directories(manager PRIVATE /usr/local/include/bsoncxx/v_noabi/bsoncxx/)
target_link_libraries(manager PRIVATE mongo::mongocxx_shared)