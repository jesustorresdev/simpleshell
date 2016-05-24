#
# CMakeLists.txt - CMake project file to build the documentation
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

#
# Build the documentation?
#
OPTION(BUILD_DOCUMENTATION "Build the documentation (Doxygen required)." OFF)
MARK_AS_ADVANCED(BUILD_DOCUMENTATION)

#
# Find Doxygen. It is required if must be added to the default target (all)
#
IF (BUILD_DOCUMENTATION)
    FIND_PACKAGE(Doxygen REQUIRED)
ELSE (BUILD_DOCUMENTATION)
    FIND_PACKAGE(Doxygen)
ENDIF (BUILD_DOCUMENTATION)

IF (DOXYGEN_FOUND)

    # configure the template doxyfile
    FOREACH(DOXYGEN_INPUT_LIST_ITEM ${DOCUMENTATION_SYSTEM_INPUT_LIST})
        SET(DOXYGEN_INPUT_LIST
            "${DOXYGEN_INPUT_LIST} ${PROJECT_SOURCE_DIR}/${DOXYGEN_INPUT_LIST_ITEM}")
    ENDFOREACH(DOXYGEN_INPUT_LIST_ITEM ${DOCUMENTATION_SYSTEM_INPUT_LIST})
    CONFIGURE_FILE(
        ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in
        ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
        @ONLY
    )
    # add a custom target to build the documentation with doxygen
    ADD_CUSTOM_TARGET(doc
        ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
        DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
        COMMENT "Generating API documentation with Doxygen" VERBATIM
    )

    IF (BUILD_DOCUMENTATION)

        # cleanup the generated documents on 'make clean'
        SET_DIRECTORY_PROPERTIES(PROPERTIES ADDITIONAL_MAKE_CLEAN_FILES html)

        # include doc target in the all target
        SET_TARGET_PROPERTIES(doc PROPERTIES EXCLUDE_FROM_ALL OFF)

    ELSE (BUILD_DOCUMENTATION)

        MESSAGE("Doxygen found. You can run 'make doc' to build the project documentation")

    ENDIF (BUILD_DOCUMENTATION)

ENDIF (DOXYGEN_FOUND)
