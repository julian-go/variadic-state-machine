#ifndef STATES_HPP_
#define STATES_HPP_

#include "vsm/vsm.hpp"

struct Event {};

struct Data {
  int on_enter_called{0};
  int on_enter_event_called{0};
  int process_called{0};
  int on_exit_called{0};
  int on_enter_A_called{0};
  int on_enter_A_event_called{0};
  int process_A_called{0};
  int on_exit_A_called{0};
  int event_handled_A{0};
  int on_enter_B_called{0};
  int on_enter_B_event_called{0};
  int process_B_called{0};
  int on_exit_B_called{0};
  int event_handled_B{0};
  char current_state = '\0';
};

bool operator==(const Data &lhs, const Data &rhs);

namespace test0 {
struct State {
  explicit State(Data &d) : data{d} {}

  void OnEnter();

  Data &data;
};
}  // namespace test0

namespace test1 {

struct State {
  explicit State(Data &d) : data{d} {}

  void OnEnter(const Event & /*event*/);
  auto Process() -> vsm::DoNothing;
  void OnExit();

  Data &data;
};

struct StateWithOnEnter : public State {
  explicit StateWithOnEnter(Data &d) : State{d} {}
  void OnEnter();
};

}  // namespace test1

namespace test2 {

struct StateB;

struct StateA {
  explicit StateA(Data &d) : data{d} {}
  void OnEnter();
  auto Process() -> vsm::Maybe<vsm::TransitionTo<StateB>>;
  void OnExit();
  auto Handle(const Event &) -> vsm::Maybe<vsm::TransitionTo<StateB>>;
  Data &data;
  bool transition_next = false;
};

struct StateB {
  explicit StateB(Data &d) : data{d} {}
  void OnEnter();
  auto Process() -> vsm::Maybe<vsm::TransitionTo<StateA>>;
  void OnExit();
  auto Handle(const Event &) -> vsm::Maybe<vsm::TransitionTo<StateA>>;
  Data &data;
  bool transition_next = false;
};

}  // namespace test2

namespace specialized {

struct StateB;

struct StateA {
  explicit StateA(Data &d) : data{d} {}
  void OnEnter();
  void OnEnter(const Event &);
  auto Process() -> vsm::Maybe<vsm::TransitionTo<StateB>>;
  void OnExit();
  auto Handle(const Event &) -> vsm::Maybe<vsm::TransitionTo<StateB>>;
  Data &data;
  bool transition_next = false;
};

struct StateB {
  explicit StateB(Data &d) : data{d} {}
  void OnEnter();
  void OnEnter(const Event &);
  auto Process() -> vsm::Maybe<vsm::TransitionTo<StateA>>;
  void OnExit();
  auto Handle(const Event &) -> vsm::Maybe<vsm::TransitionTo<StateA>>;
  Data &data;
  bool transition_next = false;
};

}  // namespace specialized

namespace specialized_no_default {

struct StateB;

struct StateA {
  explicit StateA(Data &d) : data{d} {}
  void OnEnter(const Event &);
  auto Process() -> vsm::Maybe<vsm::TransitionTo<StateB>>;
  void OnExit();
  auto Handle(const Event &) -> vsm::Maybe<vsm::TransitionTo<StateB>>;
  Data &data;
  bool transition_next = false;
};

struct StateB {
  explicit StateB(Data &d) : data{d} {}
  void OnEnter(const Event &);
  auto Process() -> vsm::Maybe<vsm::TransitionTo<StateA>>;
  void OnExit();
  auto Handle(const Event &) -> vsm::Maybe<vsm::TransitionTo<StateA>>;
  Data &data;
  bool transition_next = false;
};

}  // namespace specialized_no_default

namespace with_log {

struct StateB;

struct StateA {
  explicit StateA(Data &d) : data{d} {}
  static constexpr auto Name() { return "StateA"; }
  void OnEnter();
  auto Process() -> vsm::Maybe<vsm::TransitionTo<StateB>>;
  Data &data;
};

struct StateB {
  explicit StateB(Data &d) : data{d} {}
  static constexpr auto Name() { return "StateB"; }
  void OnEnter();
  auto Process() -> vsm::Maybe<vsm::TransitionTo<StateA>>;
  Data &data;
};

}  // namespace with_log

namespace no_log {

struct StateB;

struct StateA {
  explicit StateA(Data &d) : data{d} {}
  void OnEnter();
  auto Process() -> vsm::Maybe<vsm::TransitionTo<StateB>>;
  Data &data;
};

struct StateB {
  explicit StateB(Data &d) : data{d} {}
  void OnEnter();
  auto Process() -> vsm::Maybe<vsm::TransitionTo<StateA>>;
  Data &data;
};

}  // namespace no_log

namespace no_process {

struct StateB;

struct StateA {
  explicit StateA(Data &d) : data{d} {}
  auto Handle(const Event &) -> vsm::TransitionTo<StateB>;
  Data &data;
};

struct StateB {
  explicit StateB(Data &d) : data{d} {}
  auto Handle(const Event &) -> vsm::TransitionTo<StateA>;
  Data &data;
};

}  // namespace no_process

#endif