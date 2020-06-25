#
# Copyright 2020 Nghia Truong <nghiatruong.vn@gmail.com>
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

cmake_minimum_required(VERSION 3.5)
project(QuadraticApproximation)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_PREFIX_PATH "${PROJECT_SOURCE_DIR}/Externals/imgui" ${CMAKE_PREFIX_PATH})

set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/Modules/" ${CMAKE_MODULE_PATH})
set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/Externals/magnum-integration/modules" ${CMAKE_MODULE_PATH})

####################################################################################################
set_directory_properties(PROPERTIES CORRADE_USE_PEDANTIC_FLAGS ON)

set(WITH_RC OFF CACHE BOOL "" FORCE) # Corrade
set(BUILD_DEPRECATED OFF CACHE BOOL "" FORCE) # Corrade
set(BUILD_STATIC ON CACHE BOOL "" FORCE) # Corrade

set(MAGNUM_BUILD_DEPRECATED OFF CACHE BOOL "" FORCE) # Magnum
set(MAGNUM_BUILD_STATIC ON CACHE BOOL "" FORCE) # Magnum
set(WITH_GLFWAPPLICATION ON CACHE BOOL "" FORCE) # Magnum
set(WITH_IMGUI ON CACHE BOOL "" FORCE) # Magnum integration

set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE) # GLFW

add_subdirectory(${PROJECT_SOURCE_DIR}/Externals/corrade EXCLUDE_FROM_ALL)
add_subdirectory(${PROJECT_SOURCE_DIR}/Externals/glfw EXCLUDE_FROM_ALL)
add_subdirectory(${PROJECT_SOURCE_DIR}/Externals/magnum EXCLUDE_FROM_ALL)
add_subdirectory(${PROJECT_SOURCE_DIR}/Externals/magnum-integration EXCLUDE_FROM_ALL)

####################################################################################################
include_directories(${PROJECT_SOURCE_DIR}/Externals/ImGuizmo)
include_directories(${PROJECT_SOURCE_DIR}/Source)

find_package(Magnum REQUIRED
    GlfwApplication
    GL
    MeshTools
    Primitives
    SceneGraph
    Shaders
)
find_package(MagnumIntegration REQUIRED ImGui)

file(GLOB_RECURSE H_FILES ${PROJECT_SOURCE_DIR}/Source/*.h)
file(GLOB_RECURSE CPP_FILES ${PROJECT_SOURCE_DIR}/Source/*.cpp)

add_executable(${PROJECT_NAME}
    ${PROJECT_SOURCE_DIR}/Externals/ImGuizmo/ImGuizmo.cpp
    ${H_FILES}
    ${CPP_FILES})

target_link_libraries(${PROJECT_NAME} PRIVATE
    Magnum::Application
    Magnum::GL
    Magnum::Magnum
    Magnum::MeshTools
    Magnum::Primitives
    Magnum::SceneGraph
    Magnum::Shaders
    MagnumIntegration::ImGui)
