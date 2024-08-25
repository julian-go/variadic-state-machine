#ifndef EXAMPLE_EVENTS_H_
#define EXAMPLE_EVENTS_H_

#include <string_view>

struct FloorEvent {
  int floor;
};

struct Call : FloorEvent {
  static constexpr auto Name() -> std::string_view { return "Call"; }
};

struct FloorSensor : FloorEvent {
  static constexpr auto Name() -> std::string_view { return "FloorSensor"; }
};

struct Alarm {
  static constexpr auto Name() -> std::string_view { return "Alarm"; }
};

#endif
