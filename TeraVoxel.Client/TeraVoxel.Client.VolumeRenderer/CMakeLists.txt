cmake_minimum_required(VERSION 3.21)
project(TeraVoxel.Client.VolumeRenderer)

# Nastav statickou knihovnu
set(LIBRARY_TYPE STATIC)

# Přidej všechny zdrojové a hlavičkové soubory automaticky
file(GLOB_RECURSE LIBRARY_SOURCES "src/*")
# Vyhledání všech hlavičkových souborů
file(GLOB_RECURSE LIBRARY_HEADERS "include/*")

# Vytvoř knihovnu
add_library(TeraVoxel.Client.VolumeRenderer ${LIBRARY_TYPE} ${LIBRARY_HEADERS} ${LIBRARY_SOURCES})

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${LIBRARY_HEADERS} ${LIBRARY_SOURCES})

target_include_directories(TeraVoxel.Client.VolumeRenderer PUBLIC include)
set_target_properties(TeraVoxel.Client.VolumeRenderer
   PROPERTIES
       CUDA_SEPARABLE_COMPILATION ON
       CUDA_RESOLVE_DEVICE_SYMBOLS ON
)

find_package(CUDAToolkit)

# Závislosti
target_link_libraries(TeraVoxel.Client.VolumeRenderer PUBLIC TeraVoxel.Client.Core CUDA::cudart_static)

