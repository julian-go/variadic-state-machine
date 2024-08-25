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

  auto Handle(const Call& event,
              SmData& data) -> vsm::Maybe<vsm::TransitionTo<Moving>>;
  auto Handle(const FloorSensor& event, SmData& data) -> vsm::DoNothing;
  auto Handle(const Alarm& /*event*/,
              SmData& /*data*/) -> vsm::TransitionTo<Panic> {
    return {};
  };
};

struct Moving {
  static constexpr auto Name() -> std::string_view { return "Moving"; }

  auto Handle(const Call& event, SmData& data) -> vsm::DoNothing;
  auto Handle(const FloorSensor& event, SmData& data)
      -> vsm::Maybe<vsm::TransitionTo<Idle>, vsm::TransitionTo<Panic>>;
  auto Handle(const Alarm& /*event*/,
              SmData& /*data*/) -> vsm::TransitionTo<Panic> {
    return {};
  };
};

struct Panic {
  static constexpr auto Name() -> std::string_view { return "Panic"; }

  template <typename Event>
  auto Handle(const Event& /*event*/, SmData& /*data*/) -> vsm::DoNothing {
    // Panic does not handle any events
    return {};
  };

  void OnEnter(const FloorSensor& event, SmData& data);
  void OnEnter(const Alarm& event, SmData& data);

  void Process() {
    std::cout << "Been panicking for " << panic_seconds << " seconds"
              << std::endl;
    panic_seconds++;
  }

  int panic_seconds = 0;
};

#endif
