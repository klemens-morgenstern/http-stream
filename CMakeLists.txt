cmake_minimum_required(VERSION 3.20)

set(BOOST_HTTP_STREAM_VERSION 1)
if(BOOST_SUPERPROJECT_VERSION)
    set(BOOST_HTTP_STREAM_VERSION ${BOOST_SUPERPROJECT_VERSION})
endif()

project(boost_http_stream VERSION "${BOOST_HTTP_STREAM_VERSION}" LANGUAGES CXX)

set(BOOST_HTTP_STREAM_IS_ROOT OFF)

if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    set(BOOST_HTTP_STREAM_IS_ROOT ON)
endif()

if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/../../boost.css)
    set(BOOST_HTTP_STREAM_SHOULD_BE_INLINE ON)
else()
    set(BOOST_HTTP_STREAM_SHOULD_BE_INLINE OFF)
endif()

option(BOOST_HTTP_STREAM_BUILD_INLINE "Configure as if part of the boost source tree" ${BOOST_HTTP_STREAM_SHOULD_BE_INLINE})

if(NOT BOOST_SUPERPROJECT_VERSION)
    option(BOOST_HTTP_STREAM_INSTALL "Install boost::http_stream files" ON)
    option(BOOST_HTTP_STREAM_BUILD_TESTS "Build boost::http_stream tests" ${BUILD_TESTING})
    option(BOOST_HTTP_STREAM_BUILD_EXAMPLES "Build boost::http_stream examples" ${BOOST_HTTP_STREAM_IS_ROOT})
else()
    set(BOOST_HTTP_STREAM_BUILD_TESTS ${BUILD_TESTING})
endif()


if(BOOST_HTTP_STREAM_IS_ROOT AND BOOST_HTTP_STREAM_BUILD_INLINE)
    #
    # Building inside Boost tree, but as a separate project e.g. on Travis or
    # other CI, or when producing Visual Studio Solution and Projects.
    set(BOOST_INCLUDE_LIBRARIES http_stream)
    set(BOOST_EXCLUDE_LIBRARIES http_stream)

    set(CMAKE_FOLDER _deps)
    add_subdirectory(../.. _deps/boost EXCLUDE_FROM_ALL)
    unset(CMAKE_FOLDER)
endif()

find_package(Threads REQUIRED)

if (NOT BOOST_HTTP_STREAM_BUILD_INLINE)
    find_package(Boost REQUIRED atomic container filesystem system json url)
    include_directories(include)
endif()


add_library(boost_http_stream src/fields.cpp)
target_include_directories(boost_http_stream PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")


if (BOOST_ASYNC_BUILD_INLINE)
    target_link_libraries(
            boost_http_stream
            PUBLIC
            Boost::beast)
endif()

target_link_libraries(
        boost_http_stream
        PUBLIC
        Boost::system
        Threads::Threads)


target_compile_definitions(boost_http_stream PRIVATE BOOST_HTTP_STREAM_SOURCE=1)
add_library(Boost::http_stream ALIAS boost_http_stream)

if(BUILD_SHARED_LIBS)
    target_compile_definitions(boost_http_stream PUBLIC BOOST_HTTP_STREAM_DYN_LINK=1)
else()
    target_compile_definitions(boost_http_stream PUBLIC BOOST_HTTP_STREAM_STATIC_LINK=1)
endif()

if(BOOST_HTTP_STREAM_INSTALL AND NOT BOOST_SUPERPROJECT_VERSION)
    install(TARGETS boost_http_stream
            RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
            LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
            ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
    )
endif()

if (OPENSSL_FOUND)
    include_directories(${OPENSSL_INCLUDE_DIR})
endif()

if(BOOST_HTTP_STREAM_BUILD_TESTS)
    add_subdirectory(test)
endif()


if(BOOST_HTTP_STREAM_BUILD_EXAMPLES)
    add_subdirectory(example)
endif()