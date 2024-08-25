#include <string_view>

#include "states.hpp"
#include "vsm.hpp"

auto main() -> int {
  SmData data{};
  vsm::StateMachine sm{Idle{}, Moving{}, Panic{}};
  sm.SetLogCallback([](vsm::LogInfo info) {
    std::cout << "Transition from '" << info.from_state << "' to '"
              << info.to_state << "' due to event '" << info.event << "'"
              << std::endl;
  });

  bool running = true;
  while (running) {
    char c;
    std::cout << "Press c to call elevator, f to simulate floor sensor, a to "
                 "trigger alarm, q to quit: ";
    std::cin >> c;
    switch (c) {
      case 'c': {
        int floor;
        std::cout << "Enter floor: ";
        std::cin >> floor;
        sm.Handle(Call{floor}, data);
        break;
      }
      case 'f': {
        int floor;
        std::cout << "Enter floor: ";
        std::cin >> floor;
        sm.Handle(FloorSensor{floor}, data);
        break;
      }
      case 'a':
        sm.Handle(Alarm{}, data);
        break;
      case 'q':
        running = false;
        break;
      default:
        std::cout << "Invalid input" << std::endl;
        break;
    }
    sm.Process();
  }

  return 0;
}