# Distributed under the MIT License.
# See LICENSE.txt for details.

set(LIBRARY LinearOperators)

set(LIBRARY_SOURCES
    ApplyMatrices.cpp
    DefiniteIntegral.cpp
    Linearize.cpp
    MeanValue.cpp
    )

add_library(${LIBRARY} ${LIBRARY_SOURCES})

target_link_libraries(
  ${LIBRARY}
  INTERFACE DataStructures
  INTERFACE Domain
  INTERFACE ErrorHandling
  INTERFACE Spectral
  )
