cmake_minimum_required(VERSION 3.10)
project(xvoice_cpp_demo LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
add_executable(xvoice_cpp_demo 
    src/main.cpp 
    src/commercial/viot_dds_commecial.cpp 
    src/commercial/viot_shared_commercial.cpp
    src/viot_sqlite_wrapper.cpp
    src/commercial/viot_db_commercial.cpp
    src/commercial/viot_event_commercial.cpp
    src/commercial/viot_http_commercial.cpp
    src/commercial/viot_ntrip_commercial.cpp
    src/commercial/viot_data_center.cpp
)
target_include_directories(xvoice_cpp_demo PRIVATE include include/interface include/third_party/moodycamel include/third_party/nlohmann)
# 直接链接sqlite3库，不使用find_package
target_link_libraries(xvoice_cpp_demo sqlite3 curl pthread)