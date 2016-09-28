set(SOURCES
   ../../common/terminal_info.cpp
  terminalinfo_test.cpp
)

set(Required_LIBRARIES ${Boost_LIBRARIES} ${Boost_FRAMEWORK} -pthread)

set(TerminalinfoTest_BINARY terminalinfotest)
add_executable(${TerminalinfoTest_BINARY} ${SOURCES})
target_link_libraries(${TerminalinfoTest_BINARY} ${Required_LIBRARIES})
