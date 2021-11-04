cmake_minimum_required(VERSION 3.0.0)
project(cds_cjson VERSION 0.1.0)

include(CTest)
enable_testing()

include_directories("cJSON")
aux_source_directory(cjson_src "cjson/*.c")
add_executable(cds_cjson main.cpp ${cjson_src})
# target_link_libraries(cds_cjson PRIVATE cjson)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
