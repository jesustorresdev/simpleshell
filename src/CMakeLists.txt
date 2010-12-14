#
# CMakeLists.txt - CMake project file (src)
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

# Build the library
ADD_SUBDIRECTORY(cli)

# Build the boost::system shared library
ADD_SUBDIRECTORY(system)

# Build C++ demo
ADD_EXECUTABLE(simpleshell main.cpp)
TARGET_LINK_LIBRARIES(simpleshell cli ${DEMO_LIBS})

# Build the shared library with the C wrapper
ADD_LIBRARY(cli-c SHARED cli-wrapper.cpp)
TARGET_LINK_LIBRARIES(cli-c cli ${DEMO_LIBS})

# Build C demo
ADD_EXECUTABLE(simpleshell-c main.c)
TARGET_LINK_LIBRARIES(simpleshell-c cli-c)

# Build Pascal demo
FIND_PACKAGE(FPC REQUIRED)
IF (PASCAL_COMPILER)
    ADD_CUSTOM_COMMAND(
        OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/cliwrapper.o" "${CMAKE_CURRENT_BINARY_DIR}/cliwrapper.ppu"
        COMMAND ${PASCAL_COMPILER}
                ${PASCAL_COMPILER_FLAGS}
                -FU"${CMAKE_CURRENT_BINARY_DIR}"
                "${CMAKE_CURRENT_SOURCE_DIR}/cliwrapper.pas"
        DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/cliwrapper.pas"
    )
    ADD_CUSTOM_COMMAND(
        OUTPUT "${EXECUTABLE_OUTPUT_PATH}/simpleshell-pas"
        COMMAND ${PASCAL_COMPILER} ${PASCAL_COMPILER_FLAGS}        
                -o${EXECUTABLE_OUTPUT_PATH}/simpleshell-pas
                -Fl"${LIBRARY_OUTPUT_PATH}"
                -FU"${CMAKE_CURRENT_BINARY_DIR}"
                "${CMAKE_CURRENT_SOURCE_DIR}/main.pas"
        DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/cliwrapper.o"
                "${CMAKE_CURRENT_BINARY_DIR}/cliwrapper.ppu"
                "${CMAKE_CURRENT_SOURCE_DIR}/main.pas"
                cli-c
    )
    ADD_CUSTOM_TARGET(simpleshell-pas ALL
        DEPENDS ${EXECUTABLE_OUTPUT_PATH}/simpleshell-pas
    )
ENDIF (PASCAL_COMPILER)
