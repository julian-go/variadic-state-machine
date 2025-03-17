#include "states.hpp"

#include "vsm/vsm.hpp"

bool operator==(const Data& lhs, const Data& rhs) {
  return lhs.on_enter_called == rhs.on_enter_called &&
         lhs.on_enter_event_called == rhs.on_enter_event_called &&
         lhs.process_called == rhs.process_called &&
         lhs.on_exit_called == rhs.on_exit_called &&
         lhs.on_enter_A_called == rhs.on_enter_A_called &&
         lhs.on_enter_A_event_called == rhs.on_enter_A_event_called &&
         lhs.process_A_called == rhs.process_A_called &&
         lhs.on_exit_A_called == rhs.on_exit_A_called &&
         lhs.event_handled_A == rhs.event_handled_A &&
         lhs.on_enter_B_called == rhs.on_enter_B_called &&
         lhs.on_enter_B_event_called == rhs.on_enter_B_event_called &&
         lhs.process_B_called == rhs.process_B_called &&
         lhs.on_exit_B_called == rhs.on_exit_B_called &&
         lhs.event_handled_B == rhs.event_handled_B &&
         lhs.current_state == rhs.current_state;
}

namespace test0 {

void State::InitialTransition() { data.on_enter_called++; }

}  // namespace test0

namespace test1 {

void State::OnEnter(const Event& /*event*/) { data.on_enter_event_called++; }

auto State::Process() -> vsm::DoNothing {
  data.process_called++;
  return {};
}

void State::OnExit() { data.on_exit_called++; }

void StateWithOnEnter::OnEnter() { data.on_enter_called++; }

};  // namespace test1

namespace test2 {

void StateA::OnEnter() {
  data.on_enter_A_called++;
  transition_next = false;
  data.current_state = 'A';
}
auto StateA::Process() -> vsm::Maybe<vsm::TransitionTo<StateB>> {
  data.process_A_called++;
  if (transition_next) {
    return vsm::TransitionTo<StateB>{};
  } else {
    transition_next = true;
    return vsm::DoNothing{};
  }
}
void StateA::OnExit() { data.on_exit_A_called++; }
auto StateA::Handle(const Event& /*event*/)
    -> vsm::Maybe<vsm::TransitionTo<StateB>> {
  data.event_handled_A++;
  if (transition_next) {
    return vsm::TransitionTo<StateB>{};
  } else {
    transition_next = true;
    return vsm::DoNothing{};
  }
}

void StateB::OnEnter() {
  data.on_enter_B_called++;
  transition_next = false;
  data.current_state = 'B';
}
auto StateB::Process() -> vsm::Maybe<vsm::TransitionTo<StateA>> {
  data.process_B_called++;
  if (transition_next) {
    return vsm::TransitionTo<StateA>{};
  } else {
    transition_next = true;
    return vsm::DoNothing{};
  }
}
void StateB::OnExit() { data.on_exit_B_called++; }
auto StateB::Handle(const Event& /*event*/)
    -> vsm::Maybe<vsm::TransitionTo<StateA>> {
  data.event_handled_B++;
  if (transition_next) {
    return vsm::TransitionTo<StateA>{};
  } else {
    transition_next = true;
    return vsm::DoNothing{};
  }
}

}  // namespace test2

namespace specialized {

void StateA::OnEnter(const Event& /*event*/) {
  data.on_enter_A_event_called++;
  transition_next = false;
  data.current_state = 'A';
}
void StateA::OnEnter() {
  data.on_enter_A_called++;
  transition_next = false;
  data.current_state = 'A';
}
auto StateA::Process() -> vsm::Maybe<vsm::TransitionTo<StateB>> {
  data.process_A_called++;
  if (transition_next) {
    return vsm::TransitionTo<StateB>{};
  } else {
    transition_next = true;
    return vsm::DoNothing{};
  }
}
void StateA::OnExit() { data.on_exit_A_called++; }
auto StateA::Handle(const Event& /*event*/)
    -> vsm::Maybe<vsm::TransitionTo<StateB>> {
  data.event_handled_A++;
  if (transition_next) {
    return vsm::TransitionTo<StateB>{};
  } else {
    transition_next = true;
    return vsm::DoNothing{};
  }
}

void StateB::OnEnter(const Event& /*event*/) {
  data.on_enter_B_event_called++;
  transition_next = false;
  data.current_state = 'B';
}
void StateB::OnEnter() {
  data.on_enter_B_called++;
  transition_next = false;
  data.current_state = 'B';
}
auto StateB::Process() -> vsm::Maybe<vsm::TransitionTo<StateA>> {
  data.process_B_called++;
  if (transition_next) {
    return vsm::TransitionTo<StateA>{};
  } else {
    transition_next = true;
    return vsm::DoNothing{};
  }
}
void StateB::OnExit() { data.on_exit_B_called++; }
auto StateB::Handle(const Event& /*event*/)
    -> vsm::Maybe<vsm::TransitionTo<StateA>> {
  data.event_handled_B++;
  if (transition_next) {
    return vsm::TransitionTo<StateA>{};
  } else {
    transition_next = true;
    return vsm::DoNothing{};
  }
}

}  // namespace specialized

namespace specialized_no_default {

void StateA::OnEnter(const Event& /*event*/) {
  data.on_enter_A_event_called++;
  transition_next = false;
  data.current_state = 'A';
}

auto StateA::Process() -> vsm::Maybe<vsm::TransitionTo<StateB>> {
  data.process_A_called++;
  if (transition_next) {
    return vsm::TransitionTo<StateB>{};
  } else {
    transition_next = true;
    return vsm::DoNothing{};
  }
}
void StateA::OnExit() { data.on_exit_A_called++; }
auto StateA::Handle(const Event& /*event*/)
    -> vsm::Maybe<vsm::TransitionTo<StateB>> {
  data.event_handled_A++;
  if (transition_next) {
    return vsm::TransitionTo<StateB>{};
  } else {
    transition_next = true;
    return vsm::DoNothing{};
  }
}

void StateB::OnEnter(const Event& /*event*/) {
  data.on_enter_B_event_called++;
  transition_next = false;
  data.current_state = 'B';
}
auto StateB::Process() -> vsm::Maybe<vsm::TransitionTo<StateA>> {
  data.process_B_called++;
  if (transition_next) {
    return vsm::TransitionTo<StateA>{};
  } else {
    transition_next = true;
    return vsm::DoNothing{};
  }
}
void StateB::OnExit() { data.on_exit_B_called++; }
auto StateB::Handle(const Event& /*event*/)
    -> vsm::Maybe<vsm::TransitionTo<StateA>> {
  data.event_handled_B++;
  if (transition_next) {
    return vsm::TransitionTo<StateA>{};
  } else {
    transition_next = true;
    return vsm::DoNothing{};
  }
}

}  // namespace specialized_no_default

namespace with_log {

void StateA::OnEnter() { data.current_state = 'A'; }
auto StateA::Process() -> vsm::Maybe<vsm::TransitionTo<StateB>> {
  return vsm::TransitionTo<StateB>{};
}

void StateB::OnEnter() { data.current_state = 'B'; }
auto StateB::Process() -> vsm::Maybe<vsm::TransitionTo<StateA>> {
  return vsm::TransitionTo<StateA>{};
}

}  // namespace with_log

namespace no_log {

void StateA::OnEnter() { data.current_state = 'A'; }
auto StateA::Process() -> vsm::Maybe<vsm::TransitionTo<StateB>> {
  return vsm::TransitionTo<StateB>{};
}

void StateB::OnEnter() { data.current_state = 'B'; }
auto StateB::Process() -> vsm::Maybe<vsm::TransitionTo<StateA>> {
  return vsm::TransitionTo<StateA>{};
}

}  // namespace no_log

namespace no_process {

auto StateA::Handle(const Event& /* event */) -> vsm::TransitionTo<StateB> {
  return {};
}

auto StateB::Handle(const Event& /* event */) -> vsm::TransitionTo<StateA> {
  return {};
}

}  // namespace no_process