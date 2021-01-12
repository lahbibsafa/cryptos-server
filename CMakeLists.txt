cmake_minimum_required(VERSION 3.17)
project(server_cryptos_cloud C)
link_directories(/usr/local/lib/)
include_directories(/usr/local/opt/libmongoc/include/libbson-1.0/ /usr/local/opt/libmongoc/include/libmongoc-1.0/)

set(CMAKE_C_STANDARD 99)
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake-modules)
include(GNUInstallDirs)
include(FindZLIB)


find_package(ZLIB REQUIRED)

if (ZLIB_FOUND)
    set(LIBS ${LIBS} ${ZLIB_LIBRARIES})
    include_directories(${ZLIB_INCLUDE_DIRS})
endif ()

set(LIBS ${LIBS} ${CMAKE_THREAD_LIBS_INIT})

include(FindUlfius)
set(ULFIUS_MIN_VERSION "2.6")
find_package(Ulfius ${ULFIUS_MIN_VERSION} REQUIRED)
set(LIBS ${LIBS} ${ULFIUS_LIBRARIES} "-lorcania -ljansson")
include_directories(${ULFIUS_INCLUDE_DIRS})

include(FindYder)
find_package(Yder)
if (YDER_FOUND)
     set(LIBS ${LIBS} "-lyder")
     include_directories(${YDER_INCLUDE_DIRS})
endif()


add_executable(server_cryptos_cloud main.c functions.c functions.h database.h database.c)
target_link_libraries(server_cryptos_cloud ${LIBS})
target_link_libraries (server_cryptos_cloud bson-1.0)
target_link_libraries(server_cryptos_cloud jwt)
target_link_libraries (server_cryptos_cloud mongoc-1.0)

#target_link_libraries(server_cryptos_cloud /usr/lib/x86_64-linux-gnu/libmongoc-1.0.so.0 /usr/lib/x86_64-linux-gnu/libbson-1.0.so.0 /usr/local/lib/libjwt.so)
