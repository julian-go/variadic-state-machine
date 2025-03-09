#include <iostream>

#include "doctest.h"
#include "states.hpp"
#include "vsm/vsm.hpp"

TEST_CASE("Process called") {
  auto data = test1::Data{};

  auto sm = vsm::StateMachine(test1::StateWithOnEnter{data});

  CHECK(data.on_enter_called == 1);
  CHECK(data.on_enter_event_called == 0);
  CHECK(data.process_called == 0);
  CHECK(data.on_exit_called == 0);

  sm.Process();
  sm.Process();
  sm.Process();

  CHECK(data.on_enter_called == 1);
  CHECK(data.on_enter_event_called == 0);
  CHECK(data.process_called == 3);
  CHECK(data.on_exit_called == 0);
}

TEST_CASE("On Enter not called") {
  auto data = test1::Data{};

  auto sm = vsm::StateMachine(test1::State{data});

  CHECK(data.on_enter_called == 0);
  CHECK(data.on_enter_event_called == 0);
  CHECK(data.process_called == 0);
  CHECK(data.on_exit_called == 0);

  sm.Process();
  sm.Process();
  sm.Process();

  CHECK(data.on_enter_called == 0);
  CHECK(data.on_enter_event_called == 0);
  CHECK(data.process_called == 3);
  CHECK(data.on_exit_called == 0);
}

TEST_CASE("Process State Transition") {
  auto data = test2::Data{};

  auto sm = vsm::StateMachine(test2::StateA{data}, test2::StateB{data});

  CHECK(data.on_enter_A_called == 1);
  CHECK(data.process_A_called == 0);
  CHECK(data.current_state == 'A');

  sm.Process();

  CHECK(data.on_enter_A_called == 1);
  CHECK(data.process_A_called == 1);
  CHECK(data.current_state == 'A');

  sm.Process();

  CHECK(data.on_enter_A_called == 1);
  CHECK(data.process_A_called == 2);
  CHECK(data.on_exit_A_called == 1);
  CHECK(data.on_enter_B_called == 1);
  CHECK(data.process_B_called == 0);
  CHECK(data.current_state == 'B');

  sm.Process();

  CHECK(data.on_enter_B_called == 1);
  CHECK(data.process_B_called == 1);
  CHECK(data.current_state == 'B');

  sm.Process();

  CHECK(data.on_enter_A_called == 2);
  CHECK(data.process_A_called == 2);
  CHECK(data.on_exit_A_called == 1);
  CHECK(data.on_enter_B_called == 1);
  CHECK(data.process_B_called == 2);
  CHECK(data.on_exit_B_called == 1);
  CHECK(data.current_state == 'A');
}

TEST_CASE("Event State Transition") {
  auto data = test2::Data{};

  auto sm = vsm::StateMachine(test2::StateA{data}, test2::StateB{data});

  CHECK(data.on_enter_A_called == 1);
  CHECK(data.process_A_called == 0);
  CHECK(data.event_handled_A == 0);
  CHECK(data.current_state == 'A');

  sm.Handle(test2::Event{});

  CHECK(data.on_enter_A_called == 1);
  CHECK(data.on_enter_B_called == 0);
  CHECK(data.process_A_called == 0);
  CHECK(data.event_handled_A == 1);
  CHECK(data.current_state == 'A');

  sm.Handle(test2::Event{});

  CHECK(data.on_enter_A_called == 1);
  CHECK(data.on_enter_B_called == 1);
  CHECK(data.process_A_called == 0);
  CHECK(data.process_B_called == 0);
  CHECK(data.on_exit_A_called == 1);
  CHECK(data.event_handled_A == 2);
  CHECK(data.event_handled_B == 0);
  CHECK(data.current_state == 'B');

  sm.Handle(test2::Event{});
  sm.Handle(test2::Event{});

  CHECK(data.on_enter_A_called == 2);
  CHECK(data.on_enter_B_called == 1);
  CHECK(data.process_A_called == 0);
  CHECK(data.process_B_called == 0);
  CHECK(data.on_exit_A_called == 1);
  CHECK(data.on_exit_B_called == 1);
  CHECK(data.event_handled_A == 2);
  CHECK(data.event_handled_B == 2);
  CHECK(data.current_state == 'A');
}

TEST_CASE("Specialized Event State Transition") {
  auto data = specialized::Data{};

  auto sm =
      vsm::StateMachine(specialized::StateA{data}, specialized::StateB{data});

  CHECK(data.on_enter_A_called == 1);
  CHECK(data.on_enter_A_event_called == 0);
  CHECK(data.process_A_called == 0);
  CHECK(data.event_handled_A == 0);
  CHECK(data.current_state == 'A');

  sm.Handle(specialized::Event{});

  CHECK(data.on_enter_A_called == 1);
  CHECK(data.on_enter_B_called == 0);
  CHECK(data.on_enter_A_event_called == 0);
  CHECK(data.on_enter_B_event_called == 0);
  CHECK(data.process_A_called == 0);
  CHECK(data.event_handled_A == 1);
  CHECK(data.current_state == 'A');

  sm.Handle(specialized::Event{});

  CHECK(data.on_enter_A_called == 1);
  CHECK(data.on_enter_B_called == 0);
  CHECK(data.on_enter_A_event_called == 0);
  CHECK(data.on_enter_B_event_called == 1);
  CHECK(data.process_A_called == 0);
  CHECK(data.process_B_called == 0);
  CHECK(data.on_exit_A_called == 1);
  CHECK(data.event_handled_A == 2);
  CHECK(data.event_handled_B == 0);
  CHECK(data.current_state == 'B');

  sm.Handle(specialized::Event{});
  sm.Handle(specialized::Event{});

  CHECK(data.on_enter_A_called == 1);
  CHECK(data.on_enter_B_called == 0);
  CHECK(data.on_enter_A_event_called == 1);
  CHECK(data.on_enter_B_event_called == 1);
  CHECK(data.process_A_called == 0);
  CHECK(data.process_B_called == 0);
  CHECK(data.on_exit_A_called == 1);
  CHECK(data.on_exit_B_called == 1);
  CHECK(data.event_handled_A == 2);
  CHECK(data.event_handled_B == 2);
  CHECK(data.current_state == 'A');
}

TEST_CASE("Specialized Event State Transition without Default") {
  auto data = specialized_no_default::Data{};

  auto sm = vsm::StateMachine(specialized_no_default::StateA{data},
                              specialized_no_default::StateB{data});

  CHECK(data.on_enter_A_called == 0);
  CHECK(data.on_enter_A_event_called == 0);
  CHECK(data.process_A_called == 0);
  CHECK(data.event_handled_A == 0);
  CHECK(data.current_state == '\0');

  sm.Handle(specialized_no_default::Event{});

  CHECK(data.on_enter_A_called == 0);
  CHECK(data.on_enter_B_called == 0);
  CHECK(data.on_enter_A_event_called == 0);
  CHECK(data.on_enter_B_event_called == 0);
  CHECK(data.process_A_called == 0);
  CHECK(data.event_handled_A == 1);
  CHECK(data.current_state == '\0');

  sm.Handle(specialized_no_default::Event{});

  CHECK(data.on_enter_A_called == 0);
  CHECK(data.on_enter_B_called == 0);
  CHECK(data.on_enter_A_event_called == 0);
  CHECK(data.on_enter_B_event_called == 1);
  CHECK(data.process_A_called == 0);
  CHECK(data.process_B_called == 0);
  CHECK(data.on_exit_A_called == 1);
  CHECK(data.event_handled_A == 2);
  CHECK(data.event_handled_B == 0);
  CHECK(data.current_state == 'B');

  sm.Handle(specialized_no_default::Event{});
  sm.Handle(specialized_no_default::Event{});

  CHECK(data.on_enter_A_called == 0);
  CHECK(data.on_enter_B_called == 0);
  CHECK(data.on_enter_A_event_called == 1);
  CHECK(data.on_enter_B_event_called == 1);
  CHECK(data.process_A_called == 0);
  CHECK(data.process_B_called == 0);
  CHECK(data.on_exit_A_called == 1);
  CHECK(data.on_exit_B_called == 1);
  CHECK(data.event_handled_A == 2);
  CHECK(data.event_handled_B == 2);
  CHECK(data.current_state == 'A');
}

TEST_CASE("Logcallback executed") {
  auto data = with_log::Data{};
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
  auto data = no_log::Data{};
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