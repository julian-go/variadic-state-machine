#include <iostream>

#include "doctest.h"
#include "states.hpp"
#include "vsm/vsm.hpp"

TEST_CASE("Process called") {
  auto data = Data{};
  auto expected = data;

  auto sm = vsm::StateMachine(test1::StateWithOnEnter{data});

  expected.on_enter_called++;
  CHECK(data == expected);

  sm.Process();
  sm.Process();
  sm.Process();

  expected.process_called += 3;
  CHECK(data == expected);
}

TEST_CASE("On Enter not called") {
  auto data = Data{};
  auto expected = data;

  auto sm = vsm::StateMachine(test1::State{data});

  CHECK(data == expected);

  sm.Process();
  sm.Process();
  sm.Process();

  expected.process_called += 3;
  CHECK(data == expected);
}

TEST_CASE("Process State Transition") {
  auto data = Data{};
  auto expected = data;

  auto sm = vsm::StateMachine(test2::StateA{data}, test2::StateB{data});

  expected.on_enter_A_called++;
  expected.current_state = 'A';
  CHECK(data == expected);

  sm.Process();

  expected.process_A_called++;
  CHECK(data == expected);

  sm.Process();

  expected.process_A_called++;
  expected.on_exit_A_called++;
  expected.on_enter_B_called++;
  expected.current_state = 'B';
  CHECK(data == expected);

  sm.Process();

  expected.process_B_called++;
  CHECK(data == expected);

  sm.Process();

  expected.on_enter_A_called++;
  expected.process_B_called++;
  expected.on_exit_B_called++;
  expected.current_state = 'A';
  CHECK(data == expected);
}

TEST_CASE("Event State Transition") {
  auto data = Data{};
  auto expected = data;

  auto sm = vsm::StateMachine(test2::StateA{data}, test2::StateB{data});

  expected.on_enter_A_called++;
  expected.current_state = 'A';
  CHECK(data == expected);

  sm.Handle(Event{});

  expected.event_handled_A++;
  CHECK(data == expected);

  sm.Handle(Event{});

  expected.event_handled_A++;
  expected.on_enter_B_called++;
  expected.on_exit_A_called++;
  expected.current_state = 'B';
  CHECK(data == expected);

  sm.Handle(Event{});

  expected.event_handled_B++;
  CHECK(data == expected);

  sm.Handle(Event{});

  expected.event_handled_B++;
  expected.on_enter_A_called++;
  expected.on_exit_B_called++;
  expected.current_state = 'A';
  CHECK(data == expected);
}

TEST_CASE("Specialized Event State Transition") {
  auto data = Data{};
  auto expected = data;

  auto sm =
      vsm::StateMachine(specialized::StateA{data}, specialized::StateB{data});

  expected.on_enter_A_called++;
  expected.current_state = 'A';
  CHECK(data == expected);

  sm.Handle(Event{});

  expected.event_handled_A++;
  CHECK(data == expected);
  CHECK(sm.IsInState<specialized::StateA>());

  sm.Handle(Event{});

  expected.event_handled_A++;
  expected.on_enter_B_event_called++;
  expected.on_exit_A_called++;
  expected.current_state = 'B';
  CHECK(data == expected);
  CHECK(sm.IsInState<specialized::StateB>());

  sm.Handle(Event{});

  expected.event_handled_B++;
  CHECK(data == expected);

  sm.Handle(Event{});

  expected.event_handled_B++;
  expected.on_enter_A_event_called++;
  expected.on_exit_B_called++;
  expected.current_state = 'A';
  CHECK(data == expected);
}

TEST_CASE("Specialized Event State Transition without Default") {
  auto data = Data{};
  auto expected = data;

  auto sm = vsm::StateMachine(specialized_no_default::StateA{data},
                              specialized_no_default::StateB{data});

  CHECK(data == expected);

  sm.Handle(Event{});

  expected.event_handled_A++;
  CHECK(data == expected);

  sm.Handle(Event{});

  expected.event_handled_A++;
  expected.on_enter_B_event_called++;
  expected.on_exit_A_called++;
  expected.current_state = 'B';
  CHECK(data == expected);

  sm.Handle(Event{});

  expected.event_handled_B++;
  CHECK(data == expected);

  sm.Handle(Event{});

  expected.event_handled_B++;
  expected.on_enter_A_event_called++;
  expected.on_exit_B_called++;
  expected.current_state = 'A';
  CHECK(data == expected);
}

TEST_CASE("Logcallback executed") {
  auto data = Data{};
  int num_log_calls = 0;
  std::string from = "";
  std::string to = "";

  auto sm = vsm::StateMachine(with_log::StateA{data}, with_log::StateB{data});

  sm.SetLogCallback([&](auto from_state, auto to_state) {
    num_log_calls++;
    from = from_state;
    to = to_state;
  });

  CHECK(data.current_state == 'A');

  sm.Process();

  CHECK(data.current_state == 'B');
  CHECK(num_log_calls == 1);
  CHECK(from == "StateA");
  CHECK(to == "StateB");

  sm.Process();

  CHECK(data.current_state == 'A');
  CHECK(num_log_calls == 2);
  CHECK(from == "StateB");
  CHECK(to == "StateA");
}

TEST_CASE("Logcallback not executed") {
  auto data = Data{};
  int num_log_calls = 0;

  auto sm = vsm::StateMachine(no_log::StateA{data}, no_log::StateB{data});

  sm.SetLogCallback(
      [&](auto /* from_state */, auto /* to_state */) { num_log_calls++; });

  CHECK(data.current_state == 'A');

  sm.Process();

  CHECK(data.current_state == 'B');
  CHECK(num_log_calls == 0);

  sm.Process();

  CHECK(data.current_state == 'A');
  CHECK(num_log_calls == 0);
}

TEST_CASE("State without process call") {
  auto data = Data{};

  auto sm =
      vsm::StateMachine(no_process::StateA{data}, no_process::StateB{data});

  CHECK(sm.IsInState<no_process::StateA>());
  sm.Handle(Event{});
  CHECK(sm.IsInState<no_process::StateB>());
  sm.Handle(Event{});
  CHECK(sm.IsInState<no_process::StateA>());
  sm.Handle(Event{});
  CHECK(sm.IsInState<no_process::StateB>());
  sm.Handle(Event{});
  CHECK(sm.IsInState<no_process::StateA>());
  sm.Handle(Event{});
  CHECK(sm.IsInState<no_process::StateB>());
}

TEST_CASE("Test forward declarations") {
  struct StateB;

  struct StateA {
    static constexpr auto Name() { return "StateA"; }
    explicit StateA(Data &d) : data{d} {}
    void OnEnter() { data.on_enter_A_called++; }
    auto Process() -> vsm::DoNothing {
      data.process_A_called++;
      return {};
    }
    void OnExit() { data.on_exit_A_called++; }
    auto Handle(const Event & /* event */) -> vsm::TransitionTo<StateB> {
      data.event_handled_A++;
      return {};
    };
    Data &data;
  };

  struct StateB {
    static constexpr auto Name() { return "StateB"; }
    explicit StateB(Data &d) : data{d} {}
    void OnEnter() { data.on_enter_B_called++; }
    auto Process() -> vsm::TransitionTo<StateA> {
      data.process_B_called++;
      return {};
    }
    void OnExit() { data.on_exit_B_called++; }
    auto Handle(const Event & /* event */) -> vsm::TransitionTo<StateA> {
      data.event_handled_B++;
      return {};
    };
    Data &data;
  };

  auto data = Data{};
  auto expected = data;
  std::string from = "";
  std::string to = "";
  auto log_cb = [&](auto from_state, auto to_state) {
    from = from_state;
    to = to_state;
  };

  auto sm = vsm::StateMachine(StateA{data}, StateB{data});
  sm.SetLogCallback(log_cb);

  expected.on_enter_A_called++;
  CHECK(data == expected);

  sm.Process();

  expected.process_A_called++;
  expected.on_exit_A_called++;
  expected.on_enter_B_called++;
  CHECK(from == "StateA");
  CHECK(to == "StateB");
  CHECK(data == expected);

  sm.Handle(Event{});

  expected.event_handled_B++;
  expected.on_exit_B_called++;
  expected.on_enter_A_called++;
  CHECK(from == "StateB");
  CHECK(to == "StateA");
  CHECK(data == expected);
}