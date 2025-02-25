#include "states.hpp"

void Red::OnEnter() {
  std::cout << "Started red phase\n";
  data_.timer = 0;
  data_.to_green = true;
};

auto Red::Process() -> vsm::Maybe<vsm::TransitionTo<Yellow>> {
  data_.timer++;
  if (data_.timer > 15) {
    return vsm::TransitionTo<Yellow>{};
  } else {
    return vsm::DoNothing{};
  }
}

void Red::OnExit() { std::cout << "Done with red phase\n"; };

auto Red::Handle(const ButtonPushed& /*event*/) -> vsm::TransitionTo<Yellow> {
  return {};
}

auto Red::Handle(const Ambulance&) -> vsm::DoNothing {
  return vsm::DoNothing{};
}

void Yellow::OnEnter() {
  std::cout << "Started yellow phase\n";
  data_.timer = 0;
};

auto Yellow::Process()
    -> vsm::Maybe<vsm::TransitionTo<Red>, vsm::TransitionTo<Green>> {
  data_.timer++;
  if (data_.timer > 2) {
    if (data_.to_green) {
      return vsm::TransitionTo<Green>{};
    } else {
      return vsm::TransitionTo<Red>{};
    }
  } else {
    return vsm::DoNothing{};
  }
}

void Yellow::OnExit() { std::cout << "Done with yellow phase\n"; };

auto Yellow::Handle(const ButtonPushed&) -> vsm::DoNothing { return {}; }

auto Yellow::Handle(const Ambulance&) -> vsm::DoNothing { return {}; }

void Green::OnEnter() {
  std::cout << "Started green phase\n";
  data_.timer = 0;
  data_.to_green = false;
};

auto Green::Process() -> vsm::Maybe<vsm::TransitionTo<Yellow>> {
  data_.timer++;
  if (data_.timer > 10) {
    return vsm::TransitionTo<Yellow>{};
  } else {
    return vsm::DoNothing{};
  }
}

void Green::OnExit() { std::cout << "Done with green phase\n"; };

auto Green::Handle(const ButtonPushed&) -> vsm::DoNothing { return {}; }

auto Green::Handle(const Ambulance&) -> vsm::TransitionTo<Yellow> { return {}; }