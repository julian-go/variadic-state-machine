#include <chrono>
#include <iostream>
#include <thread>

#include "vsm/vsm.hpp"

namespace {

char input = '\0';

void GetInput() {
  while (true) {
    std::cin >> input;
    if (input == 'q') {
      break;
    }
  }
}

struct Ambulance {};

struct ButtonPushed {};

struct Data {
  int timer = 200;
  bool to_green = true;
} data;

struct Yellow;

struct Green;

struct Red {
  auto OnEnter(Data& data) {
    std::cout << "Started red phase\n";
    data.timer = 0;
    data.to_green = true;
  };

  auto Process(Data& data) -> vsm::Maybe<vsm::TransitionTo<Yellow>> {
    data.timer++;
    if (data.timer > 15) {
      return vsm::TransitionTo<Yellow>{};
    } else {
      return vsm::DoNothing{};
    }
  }

  auto OnExit(Data& /*data*/) { std::cout << "Done with red phase\n"; };

  auto Handle(Data& /*data*/, const ButtonPushed& /*event*/)
      -> vsm::TransitionTo<Yellow> {
    return {};
  }

  auto Handle(Data&, const Ambulance&) -> vsm::DoNothing {
    return vsm::DoNothing{};
  }
};

struct Yellow {
  auto OnEnter(Data& data) {
    std::cout << "Started yellow phase\n";
    data.timer = 0;
  };

  auto Process(Data& data)
      -> vsm::Maybe<vsm::TransitionTo<Red>, vsm::TransitionTo<Green>> {
    data.timer++;
    if (data.timer > 2) {
      if (data.to_green) {
        return vsm::TransitionTo<Green>{};
      } else {
        return vsm::TransitionTo<Red>{};
      }
    } else {
      return vsm::DoNothing{};
    }
  }

  auto OnExit(Data&) { std::cout << "Done with yellow phase\n"; };

  template <typename E>
  auto Handle(Data&, const E&) -> vsm::DoNothing {
    return vsm::DoNothing{};
  }
};

struct Green {
  auto OnEnter(Data& data) {
    std::cout << "Started green phase\n";
    data.timer = 0;
    data.to_green = false;
  };

  auto Process(Data& data) -> vsm::Maybe<vsm::TransitionTo<Yellow>> {
    data.timer++;
    if (data.timer > 10) {
      return vsm::TransitionTo<Yellow>{};
    } else {
      return vsm::DoNothing{};
    }
  }

  auto OnExit(Data&) { std::cout << "Done with green phase\n"; };

  template <typename E>
  auto Handle(Data&, const E&) -> vsm::DoNothing {
    return vsm::DoNothing{};
  }
};

}  // namespace

auto main() -> int {
  std::thread input_thread(GetInput);

  vsm::StateMachine sm(data, Red{}, Yellow{}, Green{});

  while (input != 'q') {
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    sm.Process(data);
    if (input == 'p') {
      input = '\0';
      sm.Handle(data, ButtonPushed{});
    }
    if (input == 'a') {
      input = '\0';
      sm.Handle(data, Ambulance{});
    }
  }

  input_thread.join();
}
