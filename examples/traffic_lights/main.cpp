#include <chrono>
#include <iostream>
#include <thread>

#include "states.hpp"
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

}  // namespace

auto main() -> int {
  std::cout << "Press 'p' to cause a transition to green, 'a' to cause a "
               "transition to red\n";

  std::thread input_thread(GetInput);

  Data data;
  vsm::StateMachine sm(Red{data}, Yellow{data}, Green{data});

  sm.InitialTransition();

  while (input != 'q') {
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    sm.Process();
    if (input == 'p') {
      input = '\0';
      sm.Handle(ButtonPushed{});
    }
    if (input == 'a') {
      input = '\0';
      sm.Handle(Ambulance{});
    }
  }

  input_thread.join();
}
