set(SOURCES
  main.cpp 
  config_parse.cpp
  websocket_interface.cpp
  global_data.cpp
  tcp_session.cpp
  tcp_server.cpp
  websocket_client.cpp
)

set(COMMONS
  ../common/terminal_info.cpp
)

#set(Required_LIBRARIES ${Boost_LIBRARIES} ${Boost_FRAMEWORK} ${Casablanca_LIBRARY} -pthread)
set(Required_LIBRARIES ${Boost_LIBRARIES} ${Boost_FRAMEWORK} -lwebsocketpp -pthread)

set(WebsocketClient_BINARY WebsocketClient)
add_executable(${WebsocketClient_BINARY} ${SOURCES} ${COMMONS})
target_link_libraries(${WebsocketClient_BINARY} ${Required_LIBRARIES})

install(
    TARGETS ${WebsocketClient_BINARY}
    RUNTIME DESTINATION bin
)