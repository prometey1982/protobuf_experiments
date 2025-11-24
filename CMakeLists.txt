cmake_minimum_required(VERSION 3.23)
project(FlasherApp VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(protobuf CONFIG REQUIRED)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

find_package(yaml-cpp REQUIRED)
find_package(libzippp REQUIRED)
find_package(Easyloggingpp REQUIRED)
find_package(SQLiteCpp REQUIRED)
find_package(Boost REQUIRED)
find_package(OpenSSL REQUIRED)

FILE(GLOB SUB_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)
FILE(GLOB HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/src/*.h*)
FILE(GLOB PROTO_FILES ${CMAKE_CURRENT_SOURCE_DIR}/proto/*.proto)

set(SOURCE
  ${SOURCE}
  ${SUB_SOURCES}
)

add_executable(${PROJECT_NAME}
    ${SOURCE}
    ${WIN32_RESOURCES}
    ${UI_HEADERS}
    ${HEADERS}
    ${PROTO_FILES}
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    protobuf::libprotobuf
    boost::boost
    openssl::openssl
)

# compile the message types
protobuf_generate(TARGET ${PROJECT_NAME} LANGUAGE cpp)
protobuf_generate(TARGET ${PROJECT_NAME} LANGUAGE python PROTOC_OUT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/python/src)
