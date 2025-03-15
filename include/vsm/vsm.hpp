// Copyright (c) 2024 Julian Gottwald
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to
// deal in the Software without restriction, including without limitation the
// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
// sell copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
// DEALINGS IN THE SOFTWARE.

#ifndef VARIADICSTATEMACHINE_VSM_H_
#define VARIADICSTATEMACHINE_VSM_H_

#include <functional>
#include <string_view>
#include <tuple>
#include <variant>

namespace vsm {

namespace detail {
template <typename T, typename = void>
constexpr bool is_complete_v = false;

template <typename T>
constexpr bool is_complete_v<T, decltype(void(sizeof(T)))> = true;

template <typename T, typename = void>
struct HasName : std::false_type {};

template <typename T>
struct HasName<
    T, std::enable_if_t<is_complete_v<T>, std::void_t<decltype(T::Name())>>>
    : std::true_type {};

template <typename, typename = std::void_t<>>
struct HasOnEnter : std::false_type {};

template <typename T>
struct HasOnEnter<
    T, std::enable_if_t<is_complete_v<T>,
                        std::void_t<decltype(std::declval<T>().OnEnter())>>>
    : std::true_type {};

}  // namespace detail

template <typename ToState>
struct TransitionTo;

/// @brief An input-driven state machine for embedded applications.
template <typename InitialState, typename... States>
class StateMachine {
 public:
  using LogCallback = std::function<void(std::string_view, std::string_view)>;

  /// @brief Constructs a new state machine.
  /// @param initial_state  The initial state the state machine begins in.
  /// @param states         The remaining states the state machine can
  /// transition to.
  explicit StateMachine(InitialState initial_state, States... states);

  /// @brief Processes the current state, calling its Process(...) function.
  /// Note: Might result in a transition.
  void Process();

  /// @brief Forwards the event to the currently active state.
  /// Note: Might result in a transition.
  template <typename Event>
  void Handle(const Event &event);

  /// @brief Checks if the state machine is currently in a specific state.
  template <typename State>
  [[nodiscard]] auto IsInState() const -> bool {
    return std::holds_alternative<State *>(current_state_);
  }

  /// @brief Sets an optional log callback that is called for every transition
  /// @param log_cb The callback to use, syntax should take (from, to)
  void SetLogCallback(LogCallback log_cb);

 private:
  template <typename ToState>
  friend struct TransitionTo;

  /// @brief Causes the statemachine to transition to a new state
  /// @tparam State     The state to transition to
  /// @return A reference to the internal state
  template <typename State>
  auto TransitionTo() -> State &;

  /// @brief The initial transition is called upon construction of the state
  /// machine
  template <typename State>
  auto InitialTransition(State &state) -> void;

  /// @brief The list of states the statemachine holds, no duplicates possible
  std::tuple<InitialState, States...> states_;

  /// @brief Holds a pointer to the currently active state
  std::variant<InitialState *, States *...> current_state_;

  /// @brief The logging callback
  LogCallback log_cb_;
};

/// @brief Defines a transition to a state, moves the statemachine to the state
/// if executed.
/// @tparam ToState     The state the transition targets
template <typename ToState>
struct TransitionTo {
  /// @brief Executes this transition
  template <typename StateMachine, typename FromState, typename... Event>
  void Execute(StateMachine &machine, FromState &from, const Event &...event);

 private:
  void Exit(...) {};
  template <typename State>
  auto Exit(State &from, ...) -> decltype(from.OnExit());
  template <typename State, typename Event>
  auto Exit(State &from, const Event &event) -> decltype(from.OnExit(event));

  void Enter(...) {};
  template <typename State>
  auto Enter(State &to, ...) -> decltype(to.OnEnter());
  template <typename State, typename Event>
  auto Enter(State &to, const Event &event) -> decltype(to.OnEnter(event));

  template <typename StateMachine, typename FromState>
  void Log(StateMachine &machine);
};

/// @brief Convenience transition that does nothing.
struct DoNothing {
  void Execute(...) const {}
};

/// @brief Convenience transition that can contain different transitions for
/// branching
/// @tparam ...Transitions  The transitions that can be contained
template <typename... Transitions>
struct Either {
  template <typename Transition>
  Either(Transition transition) : transition_{std::move(transition)} {}

  template <typename StateMachine, typename FromState, typename... Event>
  void Execute(StateMachine &machine, FromState &from, const Event &...event);

 private:
  std::variant<Transitions...> transition_;
};

/// @brief Convenience transition that can contain different transitions or
/// nothing.
/// @tparam ...Transitions  The transitions that can be contained
template <typename... Transitions>
struct Maybe : public Either<Transitions..., DoNothing> {
  using Either<Transitions..., DoNothing>::Either;
};

// =======================================================================
// Implementation of StateMachine Class
// =======================================================================

template <typename InitialState, typename... States>
StateMachine<InitialState, States...>::StateMachine(InitialState initial_state,
                                                    States... states)
    : states_{std::forward<InitialState>(initial_state),
              std::forward<States>(states)...},
      current_state_{&std::get<0>(states_)} {
  auto state_visitor = [this](auto *state) { InitialTransition(*state); };
  std::visit(state_visitor, current_state_);
}

template <typename InitialState, typename... States>
void StateMachine<InitialState, States...>::Process() {
  auto state_visitor = [this](auto *state) {
    state->Process().Execute(*this, *state);
  };
  std::visit(state_visitor, current_state_);
}

template <typename InitialState, typename... States>
template <typename Event>
void StateMachine<InitialState, States...>::Handle(const Event &event) {
  auto state_vistor = [this, &event](auto *state) -> void {
    state->Handle(event).Execute(*this, *state, event);
  };
  std::visit(state_vistor, current_state_);
}

template <typename InitialState, typename... States>
void StateMachine<InitialState, States...>::SetLogCallback(LogCallback log_cb) {
  log_cb_ = std::move(log_cb);
}

template <typename InitialState, typename... States>
template <typename State>
auto StateMachine<InitialState, States...>::TransitionTo() -> State & {
  static_assert((std::is_same_v<State, InitialState> ||
                 (std::is_same_v<State, States> || ...)),
                "Invalid state transition: State not part of state machine");
  auto &state = std::get<State>(states_);
  current_state_ = &state;
  return state;
}

template <typename InitialState, typename... States>
template <typename State>
auto StateMachine<InitialState, States...>::InitialTransition(State &state)
    -> void {
  if constexpr (detail::HasOnEnter<State>::value) {
    state.OnEnter();
  } else {
    static_assert(
        detail::is_complete_v<State>,
        "State must be fully defined before calling InitialTransition.");
  }
}

// =======================================================================
// Implementation of TransitionTo Class
// =======================================================================
template <typename ToState>
template <typename StateMachine, typename FromState, typename... Event>
void TransitionTo<ToState>::Execute(StateMachine &machine, FromState &from,
                                    const Event &...event) {
  Log<StateMachine, FromState>(machine);
  Exit(from, event...);
  auto &to = machine.template TransitionTo<ToState>();
  Enter(to, event...);
}

template <typename ToState>
template <typename State>
auto TransitionTo<ToState>::Exit(State &from, ...) -> decltype(from.OnExit()) {
  from.OnExit();
}

template <typename ToState>
template <typename State, typename Event>
auto TransitionTo<ToState>::Exit(State &from, const Event &event)
    -> decltype(from.OnExit(event)) {
  from.OnExit(event);
}

template <typename ToState>
template <typename State>
auto TransitionTo<ToState>::Enter(State &to, ...) -> decltype(to.OnEnter()) {
  to.OnEnter();
}

template <typename ToState>
template <typename State, typename Event>
auto TransitionTo<ToState>::Enter(State &to, const Event &event)
    -> decltype(to.OnEnter(event)) {
  to.OnEnter(event);
}

template <typename ToState>
template <typename StateMachine, typename FromState>
void TransitionTo<ToState>::Log(StateMachine &machine) {
  if constexpr (detail::HasName<FromState>::value &&
                detail::HasName<ToState>::value) {
    if (machine.log_cb_) {
      machine.log_cb_(FromState::Name(), ToState::Name());
    }
  } else {
    static_assert(detail::is_complete_v<FromState>,
                  "FromState must be fully defined.");
    static_assert(detail::is_complete_v<ToState>,
                  "ToState must be fully defined.");
  }
}

// =======================================================================
// Implementation of Either Class
// =======================================================================

template <typename... Transitions>
template <typename StateMachine, typename FromState, typename... Event>
void Either<Transitions...>::Execute(StateMachine &machine, FromState &from,
                                     const Event &...event) {
  auto transition_visitor = [&machine, &from,
                             &event...](auto &transition) -> void {
    transition.Execute(machine, from, event...);
  };
  std::visit(transition_visitor, transition_);
}

}  // namespace vsm

#endif
