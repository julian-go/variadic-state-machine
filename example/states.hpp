#ifndef EXAMPLE_STATES_H_
#define EXAMPLE_STATES_H_

#include <iostream>
#include <vsm.hpp>

#include "events.hpp"
#include "sm_data.hpp"

struct Idle;
struct Moving;
struct Panic;

struct Idle {
  static constexpr auto Name() -> std::string_view { return "Idle"; }

  auto Process(SmData&) -> vsm::DoNothing { return vsm::DoNothing{}; };
  auto Handle(SmData& data,
              const Call& event) -> vsm::Maybe<vsm::TransitionTo<Moving>>;
  auto Handle(SmData& data, const FloorSensor& event) -> vsm::DoNothing;
  auto Handle(SmData& /*data*/,
              const Alarm& /*event*/) -> vsm::TransitionTo<Panic> {
    return {};
  };
};

struct Moving {
  static constexpr auto Name() -> std::string_view { return "Moving"; }
  auto Process(SmData&) -> vsm::DoNothing { return vsm::DoNothing{}; };
  auto Handle(SmData& data, const Call& event) -> vsm::DoNothing;
  auto Handle(SmData& data, const FloorSensor& event)
      -> vsm::Maybe<vsm::TransitionTo<Idle>, vsm::TransitionTo<Panic>>;
  auto Handle(SmData&, const Alarm&) -> vsm::TransitionTo<Panic> { return {}; };
};

struct Panic {
  static constexpr auto Name() -> std::string_view { return "Panic"; }

  template <typename Event>
  auto Handle(SmData& /*data*/, const Event& /*event*/) -> vsm::DoNothing {
    // Panic does not handle any events
    return {};
  };

  void OnEnter(SmData& data, const FloorSensor& event);
  void OnEnter(SmData& data, const Alarm& event);

  auto Process(SmData& data) -> vsm::Maybe<vsm::TransitionTo<Idle>> {
    std::cout << "Been panicking for " << panic_seconds << " seconds"
              << std::endl;
    panic_seconds++;
    if (panic_seconds > 10) {
      return vsm::TransitionTo<Idle>{};
    }
    return vsm::DoNothing{};
  }

  int panic_seconds = 0;
};

#endif
