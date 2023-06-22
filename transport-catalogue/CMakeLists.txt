cmake_minimum_required(VERSION 3.5)

project(cpp_transport_catalogue LANGUAGES CXX)

find_package(Protobuf REQUIRED)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(PROTO_FILES
    graph.proto
    map_renderer.proto
    svg.proto
    transport_catalogue.proto
    transport_router.proto
)

protobuf_generate_cpp(PROTO_CXX_SOURCES PROTO_CXX_HEADERS
    ${PROTO_FILES}
)

add_executable(transport_catalogue
    main.cpp
    domain.h
    domain.cpp
    geo.h
    geo.cpp
    graph.h
    json.h
    json.cpp
    json_builder.h
    json_builder.cpp
    json_reader.h
    json_reader.cpp
    map_renderer.h
    map_renderer.cpp
    ranges.h
    request_handler.h
    request_handler.cpp
    router.h
    serialization.h
    serialization.cpp
    svg.h
    svg.cpp
    transport_catalogue.h
    transport_catalogue.cpp
    transport_router.h
    transport_router.cpp
    ${PROTO_FILES}
    ${PROTO_CXX_SOURCES}
    ${PROTO_CXX_HEADERS}
    ${RESOURCES}
)

target_link_libraries(transport_catalogue protobuf::libprotobuf)
target_include_directories(transport_catalogue PUBLIC ${CMAKE_CURRENT_BINARY_DIR})

target_compile_options(transport_catalogue PUBLIC
    -Wall
    -Wextra
    -Wconversion
    -Wshadow=local
 #   -Werror
)

target_compile_options(transport_catalogue PUBLIC ${warnings} -fsanitize=address)
target_link_options(transport_catalogue PUBLIC -fsanitize=address)
