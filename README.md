# Variadic State Machine
![Language](https://img.shields.io/badge/C%2B%2B-17/20-blue.svg) ![License](https://img.shields.io/badge/license-MIT-blue) [![build](https://github.com/julian-go/variadic-state-machine/actions/workflows/build.yml/badge.svg)](https://github.com/julian-go/variadic-state-machine/actions/workflows/build.yml)

A C++17 header-only state machine implementation, leveraging variadic templates for compile-time checking of transitions and event-handling.

```cpp
struct State {
  void OnEnter();
  void OnEnter(const ButtonEvent& event); // optional specialization for transition from event
  auto Process() -> Maybe<TransitionTo<OtherState>>; // might transition to 'OtherState'
  void OnExit();

  auto Handle(const Event& event) -> TransitionTo<AnotherState>; // transitions to 'AnotherState'
};

int main() {
  StateMachine sm(State{}, OtherState{}, AnotherState{});
  sm.Process(); // process currently active state
  sm.Handle(Event{}); // currently active state will receive Event
}
```

Checkout the [example](examples/traffic_lights/main.cpp).

## Build instructions
```
meson setup build
meson test --print-errorlogs -C build
```

```
meson setup build
meson compile -C build
./build/examples/traffic_lights
```

Based on [this](https://sii.pl/blog/en/implementing-a-state-machine-in-c17/) article by Michael Adamczyk.
