#include "states.hpp"

namespace {
constexpr const char* kRed =
    "\033[31m\u2B24\033[0m "
    "\033[0m\u2B24\033[0m "
    "\033[0m\u2B24\033[0m\n";
constexpr const char* kYellow =
    "\033[0m\u2B24\033[0m "
    "\033[33m\u2B24\033[0m "
    "\033[0m\u2B24\033[0m\n";
constexpr const char* kGreen =
    "\033[0m\u2B24\033[0m "
    "\033[0m\u2B24\033[0m "
    "\033[32m\u2B24\033[0m\n";
}  // namespace

void Red::OnEnter() {
  std::cout << kRed;
  data_.timer = 0;
  data_.to_green = true;
};

auto Red::Process() -> vsm::Maybe<vsm::TransitionTo<Yellow>> {
  data_.timer++;
  if (data_.timer > 5) {
    return vsm::TransitionTo<Yellow>{};
  } else {
    return vsm::DoNothing{};
  }
}

void Red::OnExit() { std::cout << "  \u2B9F  \n"; };

auto Red::Handle(const ButtonPushed& /*event*/) -> vsm::TransitionTo<Yellow> {
  std::cout << "[Event] Button pushed\n";
  return {};
}

auto Red::Handle(const Ambulance&) -> vsm::DoNothing {
  return vsm::DoNothing{};
}

void Yellow::OnEnter() {
  std::cout << kYellow;
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

void Yellow::OnExit() { std::cout << "  \u2B9F  \n"; };

auto Yellow::Handle(const ButtonPushed&) -> vsm::DoNothing { return {}; }

auto Yellow::Handle(const Ambulance&) -> vsm::DoNothing { return {}; }

void Green::OnEnter() {
  std::cout << kGreen;
  data_.timer = 0;
  data_.to_green = false;
};

auto Green::Process() -> vsm::Maybe<vsm::TransitionTo<Yellow>> {
  data_.timer++;
  if (data_.timer > 5) {
    return vsm::TransitionTo<Yellow>{};
  } else {
    return vsm::DoNothing{};
  }
}

void Green::OnExit() { std::cout << "  \u2B9F  \n"; };

auto Green::Handle(const ButtonPushed&) -> vsm::DoNothing { return {}; }

auto Green::Handle(const Ambulance&) -> vsm::TransitionTo<Yellow> {
  std::cout << "[Event] Ambulance\n";
  return {};
}