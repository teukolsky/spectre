# Distributed under the MIT License.
# See LICENSE.txt for details.

set(LIBRARY RootFinding)

set(LIBRARY_SOURCES
    QuadraticEquation.cpp
   )

add_library(${LIBRARY} ${LIBRARY_SOURCES})

target_link_libraries(
  ${LIBRARY}
  INTERFACE ErrorHandling
  )
