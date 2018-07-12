// Distributed under the MIT License.
// See LICENSE.txt for details.

#include "Evolution/Systems/GrMhd/ValenciaDivClean/Fluxes.hpp"

#include <algorithm>
#include <cstddef>

#include "DataStructures/DataVector.hpp"
#include "DataStructures/Tensor/EagerMath/DotProduct.hpp"
#include "DataStructures/Tensor/Tensor.hpp"
#include "PointwiseFunctions/GeneralRelativity/IndexManipulation.hpp"
#include "Utilities/ConstantExpressions.hpp"
#include "Utilities/Gsl.hpp"

// IWYU pragma: no_forward_declare Tensor
// IWYU pragma: no_include <array>

/// \cond
namespace grmhd {
namespace ValenciaDivClean {

void fluxes(
    const gsl::not_null<tnsr::I<DataVector, 3, Frame::Inertial>*> tilde_d_flux,
    const gsl::not_null<tnsr::I<DataVector, 3, Frame::Inertial>*>
        tilde_tau_flux,
    const gsl::not_null<tnsr::Ij<DataVector, 3, Frame::Inertial>*> tilde_s_flux,
    const gsl::not_null<tnsr::IJ<DataVector, 3, Frame::Inertial>*> tilde_b_flux,
    const gsl::not_null<tnsr::I<DataVector, 3, Frame::Inertial>*>
        tilde_phi_flux,
    const Scalar<DataVector>& tilde_d, const Scalar<DataVector>& tilde_tau,
    const tnsr::i<DataVector, 3, Frame::Inertial>& tilde_s,
    const tnsr::I<DataVector, 3, Frame::Inertial>& tilde_b,
    const Scalar<DataVector>& tilde_phi, const Scalar<DataVector>& lapse,
    const tnsr::I<DataVector, 3, Frame::Inertial>& shift,
    const Scalar<DataVector>& sqrt_det_spatial_metric,
    const tnsr::ii<DataVector, 3, Frame::Inertial>& spatial_metric,
    const tnsr::II<DataVector, 3, Frame::Inertial>& inv_spatial_metric,
    const Scalar<DataVector>& pressure,
    const tnsr::I<DataVector, 3, Frame::Inertial>& spatial_velocity,
    const Scalar<DataVector>& lorentz_factor,
    const tnsr::I<DataVector, 3, Frame::Inertial>& magnetic_field) noexcept {
  // not const to save an allocation below
  auto spatial_velocity_one_form =
      raise_or_lower_index(spatial_velocity, spatial_metric);
  const auto magnetic_field_one_form =
      raise_or_lower_index(magnetic_field, spatial_metric);
  const auto magnetic_field_dot_spatial_velocity =
      dot_product(magnetic_field, spatial_velocity_one_form);
  const auto magnetic_field_squared =
      dot_product(magnetic_field, magnetic_field_one_form);

  const DataVector one_over_w_squared = 1.0 / square(get(lorentz_factor));
  // p_star = p + p_m = p + b^2/2 = p + ((B^m v_m)^2 + (B^m B_m)/W^2)/2
  const DataVector p_star_alpha_sqrt_det_g =
      get(sqrt_det_spatial_metric) * get(lapse) *
      (get(pressure) + 0.5 * square(get(magnetic_field_dot_spatial_velocity)) +
       0.5 * get(magnetic_field_squared) * one_over_w_squared);

  // lapse b_i / W = lapse (B_i / W^2 + v_i (B^m v_m)
  tnsr::i<DataVector, 3, Frame::Inertial> lapse_b_over_w =
      std::move(spatial_velocity_one_form);
  for (size_t i = 0; i < 3; ++i) {
    lapse_b_over_w.get(i) *= get(magnetic_field_dot_spatial_velocity);
    lapse_b_over_w.get(i) +=
        one_over_w_squared * magnetic_field_one_form.get(i);
    lapse_b_over_w.get(i) *= get(lapse);
  }

  // Outside the loop to save allocations
  DataVector transport_velocity_I(p_star_alpha_sqrt_det_g.size());

  for (size_t i = 0; i < 3; ++i) {
    transport_velocity_I = get(lapse) * spatial_velocity.get(i) - shift.get(i);
    tilde_d_flux->get(i) = get(tilde_d) * transport_velocity_I;
    tilde_tau_flux->get(i) =
        get(tilde_tau) * transport_velocity_I +
        p_star_alpha_sqrt_det_g * spatial_velocity.get(i) -
        get(lapse) * get(magnetic_field_dot_spatial_velocity) * tilde_b.get(i);
    tilde_phi_flux->get(i) =
        get(lapse) * tilde_b.get(i) - get(tilde_phi) * shift.get(i);
    for (size_t j = 0; j < 3; ++j) {
      tilde_s_flux->get(i, j) = tilde_s.get(j) * transport_velocity_I -
                                lapse_b_over_w.get(j) * tilde_b.get(i);
      tilde_b_flux->get(i, j) =
          tilde_b.get(j) * transport_velocity_I +
          get(lapse) * (get(tilde_phi) * inv_spatial_metric.get(i, j) -
                        spatial_velocity.get(j) * tilde_b.get(i));
    }
    tilde_s_flux->get(i, i) += p_star_alpha_sqrt_det_g;
  }
}
}  // namespace ValenciaDivClean
}  // namespace grmhd
/// \endcond
