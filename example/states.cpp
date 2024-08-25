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

auto Idle::Handle(const Call& event,
                  SmData& data) -> vsm::Maybe<vsm::TransitionTo<Moving>> {
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

auto Idle::Handle(const FloorSensor& event,
                  SmData& /*data*/) -> vsm::DoNothing {
  std::cout << "Idle: Handle FloorSensor " << event.floor << std::endl;
  return {};
}

auto Moving::Handle(const Call& event, SmData& /*data*/) -> vsm::DoNothing {
  std::cout << "Moving: Handle FloorCall " << event.floor << std::endl;
  return {};
}

auto Moving::Handle(const FloorSensor& event, SmData& data)
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

void Panic::OnEnter(const FloorSensor& /*event*/, SmData& /*data*/) {
  CallForMaintenance();
}

void Panic::OnEnter(const Alarm& /*event*/, SmData& /*data*/) { CallForHelp(); }