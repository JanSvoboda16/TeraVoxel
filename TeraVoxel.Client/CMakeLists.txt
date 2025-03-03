cmake_minimum_required(VERSION 3.21)

project(TeraVoxel.Client LANGUAGES CXX CUDA)

set(CMAKE_CONFIGURATION_TYPES "Release;Debug" CACHE STRING "Limit build types to Release and Debug" FORCE)

# Nastavení standardů pro C++ a CUDA
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CUDA_STANDARD 17)
set(CMAKE_CUDA_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT /O2 /DNDEBUG /Ot /GL /openmp")

# Definuj minimální verzi CUDA a architektury
set(MIN_CUDA_VERSION "11.0")
set(CUDA_ARCHITECTURES "86")  # Například Turing a Ampere

# Kontrola verze CUDA
if (NOT DEFINED CMAKE_CUDA_COMPILER)
    message(FATAL_ERROR "CUDA compiler not found. Make sure you have CUDA installed and configured.")
endif()

if (CMAKE_CUDA_COMPILER_VERSION VERSION_LESS MIN_CUDA_VERSION)
    message(FATAL_ERROR "CUDA version ${CMAKE_CUDA_COMPILER_VERSION} is too low. Required version: ${MIN_CUDA_VERSION}")
endif()

# Nastav architektury CUDA
set(CMAKE_CUDA_ARCHITECTURES ${CUDA_ARCHITECTURES})
message(STATUS "Building for CUDA architectures: ${CMAKE_CUDA_ARCHITECTURES}")

set(CUDA_NVCC_FLAGS "${CUDA_NVCC_FLAGS} --allow-unsupported-compiler -Xcompiler -openmp ")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_ALLOW_COMPILER_AND_STL_VERSION_MISMATCH")

#CONAN
# Nastavení výstupního adresáře pro Conan
set(CONAN_BUILD_DIR ${CMAKE_BINARY_DIR}/conan)
include(conan.cmake)
list(APPEND CMAKE_PREFIX_PATH ${CONAN_BUILD_DIR})


foreach (TYPE ${CMAKE_CONFIGURATION_TYPES})
  # Ignore profile and detect settings from the current environment.
  conan_cmake_autodetect(BUILD_SETTINGS BUILD_TYPE ${TYPE})
  conan_cmake_install(PATH_OR_REFERENCE ${CMAKE_CURRENT_SOURCE_DIR}
                      BUILD missing
                      INSTALL_FOLDER "${CONAN_BUILD_DIR}"
                      SETTINGS ${BUILD_SETTINGS}
                      GENERATOR "cmake_find_package_multi"
  )
endforeach()


# Základní cesty
set(EIGEN_DIR "${CMAKE_SOURCE_DIR}/Dependencies/eigen-3.4.0")
set(NLOHMAN_DIR "${CMAKE_SOURCE_DIR}/Dependencies/nlohman")
set(IMGUI_DIR "${CMAKE_SOURCE_DIR}/Dependencies/ImGui")

# Přidej podsložky
add_subdirectory(TeraVoxel.Client.Core)
add_subdirectory(TeraVoxel.Client.VolumeRenderer)
add_subdirectory(TeraVoxel.Client.Gui)
