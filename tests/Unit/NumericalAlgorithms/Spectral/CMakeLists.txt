# Distributed under the MIT License.
# See LICENSE.txt for details.

set(LIBRARY "Test_Spectral")

set(LIBRARY_SOURCES
  Test_LegendreGauss.cpp
  Test_LegendreGaussLobatto.cpp
  Test_Spectral.cpp
  )

add_test_library(
  ${LIBRARY}
  "NumericalAlgorithms/Spectral/"
  "${LIBRARY_SOURCES}"
  "Spectral;Domain"
  )
