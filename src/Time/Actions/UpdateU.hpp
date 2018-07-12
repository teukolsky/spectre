// Distributed under the MIT License.
// See LICENSE.txt for details.

/// \file
/// Defines action UpdateU

#pragma once

#include <tuple>
#include <utility>

#include "DataStructures/DataBox/DataBox.hpp"
#include "DataStructures/DataBox/Prefixes.hpp"
#include "Parallel/ConstGlobalCache.hpp"
#include "Time/Tags.hpp"
#include "Time/Time.hpp"
#include "Time/TimeSteppers/TimeStepper.hpp"
#include "Utilities/Gsl.hpp"
#include "Utilities/TaggedTuple.hpp"

namespace Actions {
/// \ingroup ActionsGroup
/// \ingroup TimeGroup
/// \brief Perform variable updates for one substep
///
/// With `dt_variables_tag =
/// db::add_tag_prefix<Tags::dt, system::variables_tag>`:
///
/// Uses:
/// - ConstGlobalCache: CacheTags::TimeStepper
/// - DataBox: system::variables_tag, dt_variables_tag,
///   Tags::HistoryEvolvedVariables<system::variables_tag, dt_variables_tag>,
///   Tags::Time, Tags::TimeStep
///
/// DataBox changes:
/// - Adds: nothing
/// - Removes: nothing
/// - Modifies: system::variables_tag, dt_variables_tag,
///   Tags::HistoryEvolvedVariables<system::variables_tag, dt_variables_tag>
struct UpdateU {
  using const_global_cache_tags = tmpl::list<CacheTags::TimeStepper>;

  template <typename DbTags, typename... InboxTags, typename Metavariables,
            typename ArrayIndex, typename ActionList,
            typename ParallelComponent>
  static auto apply(db::DataBox<DbTags>& box,
                    tuples::TaggedTuple<InboxTags...>& /*inboxes*/,
                    const Parallel::ConstGlobalCache<Metavariables>& cache,
                    const ArrayIndex& /*array_index*/,
                    const ActionList /*meta*/,
                    const ParallelComponent* const /*meta*/) noexcept {
    using variables_tag = typename Metavariables::system::variables_tag;
    using dt_variables_tag = db::add_tag_prefix<Tags::dt, variables_tag>;

    db::mutate<variables_tag, dt_variables_tag,
               Tags::HistoryEvolvedVariables<variables_tag, dt_variables_tag>>(
        make_not_null(&box),
        [&cache](const auto vars, const auto dt_vars, const auto history,
                 const auto& time, const auto& time_step) noexcept {
          const auto& time_stepper =
              Parallel::get<CacheTags::TimeStepper>(cache);

          history->insert(time, *vars, std::move(*dt_vars));
          time_stepper.update_u(vars, history, time_step);
        },
        db::get<Tags::Time>(box), db::get<Tags::TimeStep>(box));

    return std::forward_as_tuple(std::move(box));
  }
};
}  // namespace Actions
