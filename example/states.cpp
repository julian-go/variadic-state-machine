#include "states.hpp"

#include <iostream>

#include "vsm.hpp"

namespace {

void CallForMaintenance() {
  std::cout << "Calling for maintenance..." << std::endl;
}

void CallForHelp() { std::cout << "Calling for help..." << std::endl; }

}  // namespace

struct Idle;
struct Moving;

auto Idle::Handle(SmData& data,
                  const Call& event) -> vsm::Maybe<vsm::TransitionTo<Moving>> {
  data.dest_floor = event.floor;

  if (data.dest_floor == data.floor) {
    return vsm::DoNothing{};
  }

  if (data.dest_floor > data.floor) {
    data.direction = 1;
  } else {
    data.direction = -1;
  }

  return vsm::TransitionTo<Moving>{};
}

auto Idle::Handle(SmData& /*data*/,
                  const FloorSensor& event) -> vsm::DoNothing {
  std::cout << "Idle: Handle FloorSensor " << event.floor << std::endl;
  return {};
}

auto Moving::Handle(SmData& /*data*/, const Call& event) -> vsm::DoNothing {
  std::cout << "Moving: Handle FloorCall " << event.floor << std::endl;
  return {};
}

auto Moving::Handle(SmData& data, const FloorSensor& event)
    -> vsm::Maybe<vsm::TransitionTo<Idle>, vsm::TransitionTo<Panic>> {
  int expected_floor = data.floor + data.direction;
  if (event.floor != expected_floor) {
    return vsm::TransitionTo<Panic>{};
  }
  data.floor = event.floor;
  if (data.floor == data.dest_floor) {
    return vsm::TransitionTo<Idle>{};
  }
  return vsm::DoNothing{};
}

void Panic::OnEnter(SmData& /*data*/, const FloorSensor& /*event*/) {
  CallForMaintenance();
}

void Panic::OnEnter(SmData& /*data*/, const Alarm& /*event*/) { CallForHelp(); }