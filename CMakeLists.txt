#
# CMakeLists.txt - CMake project file
#
#   Copyright 2010 Jesús Torres <jmtorres@ull.es>
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
INCLUDE_DIRECTORIES("${PROJECT_SOURCE_DIR}/include")
SET(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/CMakeModules")
SET(EXECUTABLE_OUTPUT_PATH "${PROJECT_BINARY_DIR}/bin")
SET(LIBRARY_OUTPUT_PATH "${PROJECT_BINARY_DIR}/lib")

SET (CLI_LINK_LIBS ${CLI_LINK_LIBS} boost_system dl)

# Find Boost. It is a requirement for the library
## TODO: The library requires all the includes but only links against
## boost_system 
#FIND_PACKAGE(Boost REQUIRED)
#IF (Boost_FOUND)
#    INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIR})
#    SET (DEMO_LIBS ${DEMO_LIBS} ${Boost_LIBRARY})
#ENDIF (Boost_FOUND)

# Compiler options
ADD_DEFINITIONS(-O0 -g -Wall -fmessage-length=0)

# Enable parser debugging
#ADD_DEFINITIONS(-DBOOST_SPIRIT_DEBUG) 

# Build all the project
ADD_SUBDIRECTORY(src)
