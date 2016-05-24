#
# CMakeLists.txt - CMake project file
#
#   Copyright 2010-2016 Jesús Torres <jmtorres@ull.es>
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
 
CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
PROJECT(simpleshell)

#
# Global configuration
#
INCLUDE_DIRECTORIES("${PROJECT_SOURCE_DIR}/include"
                    "${PROJECT_BINARY_DIR}")
SET(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/CMakeModules")
SET(EXECUTABLE_OUTPUT_PATH "${PROJECT_BINARY_DIR}/bin")
SET(LIBRARY_OUTPUT_PATH "${PROJECT_BINARY_DIR}/lib")

# Libraries to link with cli shared library  
SET(CLI_LINK_LIBS ${CLI_LINK_LIBS} dl)

# Compiler options
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0 -g -Wall -fmessage-length=0")
# Enable parser debugging
#ADD_DEFINITIONS(-DBOOST_SPIRIT_DEBUG) 

# Source files to pass to the documentation system
SET(DOCUMENTATION_SYSTEM_INPUT_LIST "src"
                                    "src/cli"
                                    "include"
                                    "include/cli"
)

#
# Enable C++11 compiler support
#
INCLUDE(CheckCXXCompilerFlag)
CHECK_CXX_COMPILER_FLAG("-std=c++11" COMPILER_SUPPORTS_CXX11)
CHECK_CXX_COMPILER_FLAG("-std=c++0x" COMPILER_SUPPORTS_CXX0X)
IF (COMPILER_SUPPORTS_CXX11)
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
ELSEIF (COMPILER_SUPPORTS_CXX0X)
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
ELSE ()
    MESSAGE(STATUS "The compiler ${CMAKE_CXX_COMPILER} has no C++11 support. Please use a different C++ compiler.")
ENDIF ()

#
# Plataform checks
#
INCLUDE(CheckFunctionExists)
CHECK_FUNCTION_EXISTS("getprogname" HAVE_GETPROGNAME)
CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/config.h.in
               ${CMAKE_CURRENT_BINARY_DIR}/config.h)

#
# Find Boost. It is a requirement for the framework
#
FIND_PACKAGE(Boost 1.44.0 REQUIRED)
IF (Boost_FOUND)
    INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIRS})
    SET (CLI_LINK_LIBS ${CLI_LINK_LIBS} ${Boost_LIBRARIES})
ENDIF ()

#
# Build the project
#
ADD_SUBDIRECTORY(include)
ADD_SUBDIRECTORY(src)
ADD_SUBDIRECTORY(doc)
