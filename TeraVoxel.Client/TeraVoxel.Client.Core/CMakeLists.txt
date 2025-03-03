cmake_minimum_required(VERSION 3.21)
project(TeraVoxel.Client.Core)

# Nastav statickou knihovnu
set(LIBRARY_TYPE STATIC)

# Přidej všechny zdrojové a hlavičkové soubory automaticky
file(GLOB_RECURSE LIBRARY_SOURCES "src/*")
# Vyhledání všech hlavičkových souborů
file(GLOB_RECURSE LIBRARY_HEADERS "include/*")

# Vytvoř knihovnu
add_library(TeraVoxel.Client.Core ${LIBRARY_TYPE} ${LIBRARY_HEADERS} ${LIBRARY_SOURCES})

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${LIBRARY_HEADERS} ${LIBRARY_SOURCES})

# Zahrnutí adresářů
target_include_directories(TeraVoxel.Client.Core PUBLIC include ${EIGEN_DIR} ${NLOHMAN_DIR})

# Závislosti
find_package(ZLIB CONFIG REQUIRED) # CONFIG IGNORES DEFAULT MODULES
find_package(httplib REQUIRED)
target_link_libraries(TeraVoxel.Client.Core PUBLIC ZLIB::ZLIB httplib::httplib)

