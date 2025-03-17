#include <iostream>

#include "doctest.h"
#include "states.hpp"
#include "vsm/vsm.hpp"

namespace test_constants {
constexpr char kStateA = 'A';
constexpr char kStateB = 'B';
constexpr std::string_view kStateAName = "StateA";
constexpr std::string_view kStateBName = "StateB";
}  // namespace test_constants

struct StateMachineFixture {
  Data data{};
  Data expected{};
  std::string from{"n/a"};
  std::string to{"n/a"};

  auto MakeLogCallback() {
    return [this](auto from_state, auto to_state) {
      from = from_state;
      to = to_state;
    };
  }
};

TEST_SUITE("Basic State Machine Functionality") {
  TEST_CASE_FIXTURE(StateMachineFixture, "Initial Transition Called") {
    auto sm = vsm::StateMachine(test0::State{data});

    expected.on_enter_called++;

    CHECK(data == expected);

    sm.Process();
    sm.Process();
    sm.Process();

    CHECK(data == expected);
  }

  TEST_CASE_FIXTURE(StateMachineFixture, "Process called") {
    auto sm = vsm::StateMachine(test1::StateWithOnEnter{data});

    CHECK(data == expected);

    sm.Process();
    sm.Process();
    sm.Process();

    expected.process_called += 3;
    CHECK(data == expected);
  }

  TEST_CASE_FIXTURE(StateMachineFixture, "State without process call") {
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

  TEST_CASE_FIXTURE(StateMachineFixture, "On Enter not called") {
    auto sm = vsm::StateMachine(test1::State{data});

    CHECK(data == expected);

    sm.Process();
    sm.Process();
    sm.Process();

    expected.process_called += 3;
    CHECK(data == expected);
  }
}

TEST_SUITE("State Transitions") {
  TEST_CASE_FIXTURE(StateMachineFixture, "Process State Transition") {
    auto sm = vsm::StateMachine(test2::StateA{data}, test2::StateB{data});

    CHECK(data == expected);

    sm.Process();

    expected.process_A_called++;
    CHECK(data == expected);

    sm.Process();

    expected.process_A_called++;
    expected.on_exit_A_called++;
    expected.on_enter_B_called++;
    expected.current_state = test_constants::kStateB;
    CHECK(data == expected);

    sm.Process();

    expected.process_B_called++;
    CHECK(data == expected);

    sm.Process();

    expected.on_enter_A_called++;
    expected.process_B_called++;
    expected.on_exit_B_called++;
    expected.current_state = test_constants::kStateA;
    CHECK(data == expected);
  }

  TEST_CASE_FIXTURE(StateMachineFixture, "Event State Transition") {
    auto sm = vsm::StateMachine(test2::StateA{data}, test2::StateB{data});

    CHECK(data == expected);

    sm.Handle(Event{});

    expected.event_handled_A++;
    CHECK(data == expected);

    sm.Handle(Event{});

    expected.event_handled_A++;
    expected.on_enter_B_called++;
    expected.on_exit_A_called++;
    expected.current_state = test_constants::kStateB;
    CHECK(data == expected);

    sm.Handle(Event{});

    expected.event_handled_B++;
    CHECK(data == expected);

    sm.Handle(Event{});

    expected.event_handled_B++;
    expected.on_enter_A_called++;
    expected.on_exit_B_called++;
    expected.current_state = test_constants::kStateA;
    CHECK(data == expected);
  }

  TEST_CASE_FIXTURE(StateMachineFixture, "Specialized Event State Transition") {
    auto sm =
        vsm::StateMachine(specialized::StateA{data}, specialized::StateB{data});

    CHECK(data == expected);

    sm.Handle(Event{});

    expected.event_handled_A++;
    CHECK(data == expected);
    CHECK(sm.IsInState<specialized::StateA>());

    sm.Handle(Event{});

    expected.event_handled_A++;
    expected.on_enter_B_event_called++;
    expected.on_exit_A_called++;
    expected.current_state = test_constants::kStateB;
    CHECK(data == expected);
    CHECK(sm.IsInState<specialized::StateB>());

    sm.Handle(Event{});

    expected.event_handled_B++;
    CHECK(data == expected);

    sm.Handle(Event{});

    expected.event_handled_B++;
    expected.on_enter_A_event_called++;
    expected.on_exit_B_called++;
    expected.current_state = test_constants::kStateA;
    CHECK(data == expected);
  }

  TEST_CASE_FIXTURE(StateMachineFixture,
                    "Specialized Event State Transition without Default") {
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
    expected.current_state = test_constants::kStateB;
    CHECK(data == expected);

    sm.Handle(Event{});

    expected.event_handled_B++;
    CHECK(data == expected);

    sm.Handle(Event{});

    expected.event_handled_B++;
    expected.on_enter_A_event_called++;
    expected.on_exit_B_called++;
    expected.current_state = test_constants::kStateA;
    CHECK(data == expected);
  }
}

TEST_SUITE("Logging") {
  TEST_CASE_FIXTURE(StateMachineFixture, "Logcallback executed") {
    int num_log_calls = 0;
    auto sm = vsm::StateMachine(with_log::StateA{data}, with_log::StateB{data});

    sm.SetLogCallback([&](auto from_state, auto to_state) {
      num_log_calls++;
      from = from_state;
      to = to_state;
    });

    sm.Process();

    CHECK(data.current_state == test_constants::kStateB);
    CHECK(num_log_calls == 1);
    CHECK(from == test_constants::kStateAName);
    CHECK(to == test_constants::kStateBName);

    sm.Process();

    CHECK(data.current_state == test_constants::kStateA);
    CHECK(num_log_calls == 2);
    CHECK(from == test_constants::kStateBName);
    CHECK(to == test_constants::kStateAName);
  }

  TEST_CASE_FIXTURE(StateMachineFixture, "Logcallback not executed") {
    int num_log_calls = 0;
    auto sm = vsm::StateMachine(no_log::StateA{data}, no_log::StateB{data});

    sm.SetLogCallback(
        [&](auto /* from_state */, auto /* to_state */) { num_log_calls++; });

    sm.Process();

    CHECK(data.current_state == test_constants::kStateB);
    CHECK(num_log_calls == 0);

    sm.Process();

    CHECK(data.current_state == test_constants::kStateA);
    CHECK(num_log_calls == 0);
  }
}

TEST_CASE_FIXTURE(StateMachineFixture, "Test forward declarations") {
  struct StateB;

  struct StateA {
    static constexpr auto Name() { return test_constants::kStateAName; }
    explicit StateA(Data &d) : data{d} {}
    void InitialTransition() { OnEnter(); }
    void OnEnter() { data.on_enter_A_called++; }
    auto Process() -> vsm::TransitionTo<StateB> {
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
    static constexpr auto Name() { return test_constants::kStateBName; }
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
  CHECK(from == test_constants::kStateAName);
  CHECK(to == test_constants::kStateBName);
  CHECK(data == expected);

  sm.Handle(Event{});

  expected.event_handled_B++;
  expected.on_exit_B_called++;
  expected.on_enter_A_called++;
  CHECK(from == test_constants::kStateBName);
  CHECK(to == test_constants::kStateAName);
  CHECK(data == expected);
}