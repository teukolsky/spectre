# Distributed under the MIT License.
# See LICENSE.txt for details.

set(LIBRARY "Test_GeneralRelativity")

set(LIBRARY_SOURCES
  GrTestHelpers.cpp
  Test_Christoffel.cpp
  Test_ComputeGhQuantities.cpp
  Test_ComputeSpacetimeQuantities.cpp
  Test_IndexManipulation.cpp
  Test_Ricci.cpp
  )

add_test_library(
  ${LIBRARY}
  "PointwiseFunctions/GeneralRelativity/"
  "${LIBRARY_SOURCES}"
  "GeneralRelativity;Test_Pypp"
  )
