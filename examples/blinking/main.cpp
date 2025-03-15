#include <iostream>

#include "vsm/vsm.hpp"

struct LightOff;
struct LightOn;
struct SwitchPressed {};

struct LightOff {
  auto Handle(const SwitchPressed &) -> vsm::TransitionTo<LightOn> {
    std::cout << "Light is now on\n";
    return {};
  }
};

struct LightOn {
  auto Handle(const SwitchPressed &) -> vsm::TransitionTo<LightOff> {
    std::cout << "Light is now off\n";
    return {};
  }
};

int main() {
  vsm::StateMachine sm(LightOff{}, LightOn{});

  sm.Handle(SwitchPressed{});
  // Light is now on

  sm.Handle(SwitchPressed{});
  // Light is now off

  sm.Handle(SwitchPressed{});
  // Light is now on

  return 0;
}