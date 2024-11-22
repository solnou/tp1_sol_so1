cmake_minimum_required(VERSION 3.10)
project(tp1_sol_so1)

# Añadir el directorio donde se instalan las librerías compartidas (instaladas con sudo make install)
link_directories(/usr/local/lib)

# Encontrar el paquete cJSON
find_package(cJSON REQUIRED)

# Establecer el tipo de construcción por defecto
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release) 
endif()

include(../../build/Release/generators/conan_toolchain.cmake)
# Add subdirectories
add_subdirectory(lib/prometheus-client-c/prom)
add_subdirectory(lib/prometheus-client-c/promhttp)


# Incluir directorios de encabezados
include_directories(${CMAKE_SOURCE_DIR}/include)
include_directories(${CMAKE_SOURCE_DIR}/lib)
include_directories(${CONAN_INCLUDE_DIRS}) 

# Añadir el ejecutable
add_executable(tp1_sol_so1
    src/main.c
    src/metrics.c
    src/expose_metrics.c
    src/write_json.c
)

add_library( tp1_sol_so1_lib STATIC
    src/metrics.c
    src/expose_metrics.c
    src/write_json.c
    include/metrics.h
    include/expose_metrics.h
    include/write_json.h
)

# Añadir el directorio donde se instalan las librerías compartidas (instaladas con sudo make install)
link_directories(/usr/local/lib)

target_link_libraries(tp1_sol_so1 PRIVATE tp1_sol_so1_lib)


find_package(cJSON REQUIRED)

# Vincular las librerías libprom y libpromhttp desde /usr/local/lib
target_link_libraries(tp1_sol_so1_lib
    /usr/local/lib/libprom.so
    /usr/local/lib/libpromhttp.so
    cjson::cjson
)
