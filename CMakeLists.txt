cmake_minimum_required(VERSION 3.16)
project(Arise VERSION 1.0 LANGUAGES CXX)

# ============================================================
# C++ 标准
# ============================================================
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ============================================================
# 查找依赖库
# ============================================================
find_package(Protobuf REQUIRED)
find_package(Threads REQUIRED)

# jsoncpp 和 curl 只有 login 需要
find_package(PkgConfig)
if(PkgConfig_FOUND)
    pkg_check_modules(JSONCPP jsoncpp)
    pkg_check_modules(CURL libcurl)
endif()

# 兜底: 如果 pkg-config 没找到, 尝试手动指定
if(NOT JSONCPP_FOUND)
    find_path(JSONCPP_INCLUDE_DIRS json/json.h
              PATHS /usr/include/jsoncpp)
    find_library(JSONCPP_LIBRARIES jsoncpp)
    if(JSONCPP_INCLUDE_DIRS AND JSONCPP_LIBRARIES)
        set(JSONCPP_FOUND TRUE)
        message(STATUS "Found jsoncpp: ${JSONCPP_INCLUDE_DIRS}, ${JSONCPP_LIBRARIES}")
    endif()
endif()

if(NOT CURL_FOUND)
    find_package(CURL)
endif()

# ============================================================
# 核心服务端库: arise_server (static)
# ============================================================
set(LIBSERVER_DIR ${CMAKE_SOURCE_DIR}/src/libs/libserver)

set(LIBSERVER_SOURCES
    ${LIBSERVER_DIR}/base_buffer.cpp
    ${LIBSERVER_DIR}/connect_obj.cpp
    ${LIBSERVER_DIR}/console.cpp
    ${LIBSERVER_DIR}/global.cpp
    ${LIBSERVER_DIR}/message_list.cpp
    ${LIBSERVER_DIR}/network.cpp
    ${LIBSERVER_DIR}/network_buffer.cpp
    ${LIBSERVER_DIR}/network_connector.cpp
    ${LIBSERVER_DIR}/network_listen.cpp
    ${LIBSERVER_DIR}/packet.cpp
    ${LIBSERVER_DIR}/server_app.cpp
    ${LIBSERVER_DIR}/thread.cpp
    ${LIBSERVER_DIR}/thread_mgr.cpp
    ${LIBSERVER_DIR}/thread_obj.cpp
    ${LIBSERVER_DIR}/object_block.cpp
    ${LIBSERVER_DIR}/object_pool_mgr.cpp
    ${LIBSERVER_DIR}/console_cmd_pool.cpp
    ${LIBSERVER_DIR}/robot_test.cpp
    # 预生成的 protobuf 文件
    ${LIBSERVER_DIR}/protobuf/msg.pb.cc
    ${LIBSERVER_DIR}/protobuf/proto_id.pb.cc
)

set(LIBSERVER_HEADERS
    ${LIBSERVER_DIR}/base_buffer.h
    ${LIBSERVER_DIR}/common.h
    ${LIBSERVER_DIR}/connect_obj.h
    ${LIBSERVER_DIR}/console.h
    ${LIBSERVER_DIR}/disposable.h
    ${LIBSERVER_DIR}/global.h
    ${LIBSERVER_DIR}/message_list.h
    ${LIBSERVER_DIR}/network.h
    ${LIBSERVER_DIR}/network_buffer.h
    ${LIBSERVER_DIR}/network_connector.h
    ${LIBSERVER_DIR}/network_listen.h
    ${LIBSERVER_DIR}/packet.h
    ${LIBSERVER_DIR}/proto.h
    ${LIBSERVER_DIR}/robot_state_type.h
    ${LIBSERVER_DIR}/robot_test.h
    ${LIBSERVER_DIR}/server_app.h
    ${LIBSERVER_DIR}/singleton.h
    ${LIBSERVER_DIR}/sn_object.h
    ${LIBSERVER_DIR}/socket_object.h
    ${LIBSERVER_DIR}/state_template.h
    ${LIBSERVER_DIR}/thread.h
    ${LIBSERVER_DIR}/thread_mgr.h
    ${LIBSERVER_DIR}/thread_obj.h
    ${LIBSERVER_DIR}/util_string.h
    ${LIBSERVER_DIR}/util_time.h
    ${LIBSERVER_DIR}/protobuf/msg.pb.h
    ${LIBSERVER_DIR}/protobuf/proto_id.pb.h
)

add_library(arise_server STATIC ${LIBSERVER_SOURCES} ${LIBSERVER_HEADERS})

target_include_directories(arise_server
    PUBLIC
        ${CMAKE_SOURCE_DIR}/src
        ${CMAKE_SOURCE_DIR}/src/libs
        ${LIBSERVER_DIR}
        ${LIBSERVER_DIR}/protobuf
        ${Protobuf_INCLUDE_DIRS}
)

target_link_libraries(arise_server
    PUBLIC
        ${Protobuf_LIBRARIES}
        Threads::Threads
)

# ============================================================
# 登录服务: login
# ============================================================
set(LOGIN_DIR ${CMAKE_SOURCE_DIR}/src/apps/login)

add_executable(login
    ${LOGIN_DIR}/account.cpp
    ${LOGIN_DIR}/http_request.cpp
    ${LOGIN_DIR}/http_request_account.cpp
    ${LOGIN_DIR}/login_app.cpp
    ${LOGIN_DIR}/login_obj.cpp
    ${LOGIN_DIR}/login_obj_mgr.cpp
    ${LOGIN_DIR}/main.cpp
)

target_include_directories(login
    PRIVATE
        ${LOGIN_DIR}
        ${JSONCPP_INCLUDE_DIRS}
        ${CURL_INCLUDE_DIRS}
)

target_link_libraries(login
    arise_server
    ${JSONCPP_LIBRARIES}
    ${CURL_LIBRARIES}
)

# ============================================================
# 机器人测试工具: robots
# ============================================================
set(ROBOTS_DIR ${CMAKE_SOURCE_DIR}/src/tools/robots)

add_executable(robots
    ${ROBOTS_DIR}/global_robots.cpp
    ${ROBOTS_DIR}/robot.cpp
    ${ROBOTS_DIR}/robot_console.cpp
    ${ROBOTS_DIR}/robot_mgr.cpp
    ${ROBOTS_DIR}/robot_state.cpp
    ${ROBOTS_DIR}/robot_state_login.cpp
    ${ROBOTS_DIR}/robots_app.cpp
    ${ROBOTS_DIR}/main.cpp
)

target_include_directories(robots
    PRIVATE
        ${ROBOTS_DIR}
)

target_link_libraries(robots
    arise_server
)

# ============================================================
# 编译输出到 bin/
# ============================================================
set_target_properties(login robots PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin
)
