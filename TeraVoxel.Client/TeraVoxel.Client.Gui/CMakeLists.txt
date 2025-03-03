cmake_minimum_required(VERSION 3.21)
project(TeraVoxel.Client.Gui)

# Přidej všechny zdrojové soubory automaticky
file(GLOB_RECURSE APP_SOURCES "src/*")
# Vyhledání všech hlavičkových souborů
file(GLOB_RECURSE APP_HEADERS "include/*")

file(GLOB_RECURSE IMGUI_SOURCES "${IMGUI_DIR}/*") # TODO: NASTY SOLUTION

include_directories(TeraVoxel.Client.Gui PRIVATE include ${IMGUI_DIR} )

# Vytvoř spustitelný soubor
add_executable(TeraVoxel.Client.Gui ${APP_SOURCES} ${APP_HEADERS} ${IMGUI_SOURCES})

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${APP_HEADERS} ${APP_SOURCES}) 

target_link_libraries(TeraVoxel.Client.Gui PRIVATE TeraVoxel.Client.VolumeRenderer d3d11 dxgi)


set_target_properties(TeraVoxel.Client.Gui PROPERTIES CUDA_RESOLVE_DEVICE_SYMBOLS ON)

