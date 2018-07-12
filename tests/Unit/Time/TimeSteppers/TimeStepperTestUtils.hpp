// Distributed under the MIT License.
// See LICENSE.txt for details.

#pragma once

#include "tests/Unit/TestingFramework.hpp"

#include <cmath>
#include <deque>
#include <tuple>

#include "ErrorHandling/Assert.hpp"
#include "Time/BoundaryHistory.hpp"
#include "Time/History.hpp"
#include "Time/Slab.hpp"
#include "Time/Time.hpp"
#include "Time/TimeId.hpp"
#include "Utilities/ConstantExpressions.hpp"
#include "Utilities/Gsl.hpp"

namespace TimeStepperTestUtils {

template <typename Stepper, typename F>
void take_step(
    const gsl::not_null<Time*> time,
    const gsl::not_null<double*> y,
    const gsl::not_null<TimeSteppers::History<double, double>*> history,
    const Stepper& stepper,
    F&& rhs,
    const TimeDelta& step_size) noexcept {
  TimeId time_id(step_size.is_positive(), 0, *time);
  for (size_t substep = 0; substep < stepper.number_of_substeps(); ++substep) {
    CHECK(time_id.substep() == substep);
    history->insert(time_id.time(), *y, rhs(*y));
    stepper.update_u(y, history, step_size);
    time_id = stepper.next_time_id(time_id, step_size);
  }
  CHECK(time_id.time() - *time == step_size);
  *time = time_id.time();
}

template <typename Stepper, typename F1, typename F2>
void initialize_history(
    Time time,
    const gsl::not_null<TimeSteppers::History<double, double>*> history,
    const Stepper& stepper,
    F1&& analytic,
    F2&& rhs,
    TimeDelta step_size) noexcept {
  for (size_t j = 0; j < stepper.number_of_past_steps(); ++j) {
    ASSERT(time.slab() == step_size.slab(), "Slab mismatch");
    if ((step_size.is_positive() and time.is_at_slab_start()) or
        (not step_size.is_positive() and time.is_at_slab_end())) {
      const Slab new_slab = time.slab().advance_towards(-step_size);
      time = time.with_slab(new_slab);
      step_size = step_size.with_slab(new_slab);
    }
    time -= step_size;
    history->insert_initial(time, analytic(time.value()),
                            rhs(analytic(time.value())));
  }
}

template <typename Stepper>
void check_multistep_properties(const Stepper& stepper) noexcept {
  CHECK(stepper.number_of_substeps() == 1);
}

template <typename Stepper>
void check_substep_properties(const Stepper& stepper) noexcept {
  CHECK(stepper.is_self_starting());
  CHECK(stepper.number_of_past_steps() == 0);
}

template <typename Stepper>
void integrate_test(const Stepper& stepper, const double integration_time,
                    const double epsilon) noexcept {
  auto analytic = [](double t) { return sin(t); };
  auto rhs = [](double v) { return sqrt(1. - square(v)); };

  const size_t num_steps = 800;
  const Slab slab = integration_time > 0
      ? Slab::with_duration_from_start(0., integration_time)
      : Slab::with_duration_to_end(0., -integration_time);
  const TimeDelta step_size = integration_time > 0
      ? slab.duration() / num_steps
      : -slab.duration() / num_steps;

  Time time = integration_time > 0 ? slab.start() : slab.end();
  double y = analytic(time.value());
  TimeSteppers::History<double, double> history;

  if (not stepper.is_self_starting()) {
    initialize_history(time, &history, stepper, analytic, rhs, step_size);
  }

  for (size_t i = 0; i < num_steps; ++i) {
    take_step(&time, &y, &history, stepper, rhs, step_size);
    // This check needs a looser tolerance for lower-order time steppers.
    CHECK(y == approx(analytic(time.value())).epsilon(epsilon));
  }
  // Make sure history is being cleaned up.  The limit of 20 is
  // arbitrary, but much larger than the order of any integrators we
  // care about and much smaller than the number of time steps in the
  // test.
  CHECK(history.size() < 20);
}

template <typename Stepper>
void integrate_variable_test(const Stepper& stepper,
                             const double epsilon) noexcept {
  auto analytic = [](double t) { return sin(t); };
  auto rhs = [](double v) { return sqrt(1. - square(v)); };

  const size_t num_steps = 800;
  const double average_step = 1. / num_steps;

  Slab slab = Slab::with_duration_to_end(0., average_step);
  Time time = slab.end();
  double y = analytic(time.value());

  TimeSteppers::History<double, double> history;
  if (not stepper.is_self_starting()) {
    initialize_history(time, &history, stepper, analytic, rhs, slab.duration());
  }

  for (size_t i = 0; i < num_steps; ++i) {
    slab = slab.advance().with_duration_from_start(
        (1. + 0.5 * sin(i)) * average_step);

    take_step(&time, &y, &history, stepper, rhs, slab.duration());
    // This check needs a looser tolerance for lower-order time steppers.
    CHECK(y == approx(analytic(time.value())).epsilon(epsilon));
  }
}

template <typename Stepper>
void stability_test(const Stepper& stepper) noexcept {
  const size_t num_steps = 5000;
  const double bracket_size = 1.1;

  // This is integrating dy/dt = -2y, which is chosen so that the stable
  // step size for Euler's method is 1.

  // Stable region
  {
    const Slab slab = Slab::with_duration_from_start(
        0., num_steps * stepper.stable_step() / bracket_size);
    const TimeDelta step_size = slab.duration() / num_steps;

    Time time = slab.start();
    double y = 1.;
    TimeSteppers::History<double, double> history;
    if (not stepper.is_self_starting()) {
      initialize_history(time, &history, stepper,
                         [](double t) { return exp(-2. * t); },
                         [](double v) { return -2. * v; },
                         step_size);
    }

    for (size_t i = 0; i < num_steps; ++i) {
      take_step(&time, &y, &history, stepper, [](double v) { return -2. * v; },
                step_size);
      CHECK(std::abs(y) < 10.);
    }
  }

  // Unstable region
  {
    const Slab slab = Slab::with_duration_from_start(
        0., num_steps * stepper.stable_step() * bracket_size);
    const TimeDelta step_size = slab.duration() / num_steps;

    Time time = slab.start();
    double y = 1.;
    TimeSteppers::History<double, double> history;
    if (not stepper.is_self_starting()) {
      initialize_history(time, &history, stepper,
                         [](double t) { return exp(-2. * t); },
                         [](double v) { return -2. * v; },
                         step_size);
    }

    for (size_t i = 0; i < num_steps; ++i) {
      take_step(&time, &y, &history, stepper, [](double v) { return -2. * v; },
                step_size);
      if (std::abs(y) > 10.) {
        return;
      }
    }
    CHECK(false);
  }
}

template <typename Stepper>
void equal_rate_boundary(const Stepper& stepper, const double epsilon,
                         const bool forward) {
  // This does an integral putting the entire derivative into the
  // boundary term.
  const double unused_local_deriv = 4444.;

  auto analytic = [](double t) { return sin(t); };
  auto driver = [](double t) { return cos(t); };
  auto coupling = [=](const double& local, const double& remote) {
    CHECK(local == unused_local_deriv);
    return remote;
  };

  Approx approx = Approx::custom().epsilon(epsilon);

  const size_t num_steps = 100;
  const Slab slab(0.875, 1.);
  const TimeDelta step_size = (forward ? 1 : -1) * slab.duration() / num_steps;

  TimeId time_id(forward, 0, forward ? slab.start() : slab.end());
  double y = analytic(time_id.time().value());
  TimeSteppers::History<double, double> volume_history;
  TimeSteppers::BoundaryHistory<double, double, double> boundary_history;

  if (not stepper.is_self_starting()) {
    Time history_time = time_id.time();
    TimeDelta history_step_size = step_size;
    for (size_t j = 0; j < stepper.number_of_past_steps(); ++j) {
      ASSERT(history_time.slab() == history_step_size.slab(), "Slab mismatch");
      if ((history_step_size.is_positive() and
           history_time.is_at_slab_start()) or
          (not history_step_size.is_positive() and
           history_time.is_at_slab_end())) {
        const Slab new_slab =
            history_time.slab().advance_towards(-history_step_size);
        history_time = history_time.with_slab(new_slab);
        history_step_size = history_step_size.with_slab(new_slab);
      }
      history_time -= history_step_size;
      volume_history.insert_initial(history_time,
                                    analytic(history_time.value()), 0.);
      boundary_history.local_insert_initial(history_time, unused_local_deriv);
      boundary_history.remote_insert_initial(history_time,
                                             driver(history_time.value()));
    }
  }

  for (size_t i = 0; i < num_steps; ++i) {
    for (size_t substep = 0;
         substep < stepper.number_of_substeps();
         ++substep) {
      volume_history.insert(time_id.time(), y, 0.);
      boundary_history.local_insert(time_id.time(), unused_local_deriv);
      boundary_history.remote_insert(time_id.time(),
                                     driver(time_id.time().value()));

      stepper.update_u(make_not_null(&y), make_not_null(&volume_history),
                       step_size);
      y += stepper.compute_boundary_delta(
          coupling, make_not_null(&boundary_history), step_size);
      time_id = stepper.next_time_id(time_id, step_size);
    }
    CHECK(y == approx(analytic(time_id.time().value())));
  }
  // Make sure history is being cleaned up.  The limit of 20 is
  // arbitrary, but much larger than the order of any integrators we
  // care about and much smaller than the number of time steps in the
  // test.
  CHECK(boundary_history.local_size() < 20);
  CHECK(boundary_history.remote_size() < 20);
}

}  // namespace TimeStepperTestUtils
