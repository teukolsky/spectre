# Distributed under the MIT License.
# See LICENSE.txt for details.

set(LIBRARY "Test_NewtonianEuler")

set(LIBRARY_SOURCES
  Test_Characteristics.cpp
  Test_ConservativeFromPrimitive.cpp
  Test_Fluxes.cpp
  Test_PrimitiveFromConservative.cpp
  )

add_subdirectory(NewtonianEulerSources)

add_test_library(
  ${LIBRARY}
  "Evolution/Systems/NewtonianEuler/"
  "${LIBRARY_SOURCES}"
  "NewtonianEuler;Test_Domain"
  )
