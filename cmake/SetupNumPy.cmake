# Distributed under the MIT License.
# See LICENSE.txt for details.

find_package(NumPy REQUIRED)

message(STATUS "NumPy incl: " ${NUMPY_INCLUDE_DIRS})
message(STATUS "NumPy vers: " ${NUMPY_VERSION})

file(APPEND
  "${CMAKE_BINARY_DIR}/LibraryVersions.txt"
  "NumPy Version:  ${NUMPY_VERSION}\n"
  )
