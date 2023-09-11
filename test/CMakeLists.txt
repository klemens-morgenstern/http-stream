
file(GLOB_RECURSE ALL_TEST_FILES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)

add_executable(boost_http_stream_tests ${ALL_TEST_FILES})
target_link_libraries(boost_http_stream_tests PUBLIC Boost::http_stream rt)
