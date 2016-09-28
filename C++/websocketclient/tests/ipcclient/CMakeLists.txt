set(SOURCES
  ipc_client.cpp
)

set(Required_LIBRARIES ${Boost_LIBRARIES} ${Boost_FRAMEWORK} -pthread)

set(IpcClient_BINARY ipcclient)
add_executable(${IpcClient_BINARY} ${SOURCES})
target_link_libraries(${IpcClient_BINARY} ${Required_LIBRARIES})
