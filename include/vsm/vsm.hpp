#ifndef VARIADICSTATEMACHINE_VSM_H_
#define VARIADICSTATEMACHINE_VSM_H_

#include <functional>
#include <string_view>
#include <tuple>
#include <variant>

namespace vsm {

namespace detail {
template <typename Type, typename = void>
constexpr bool is_complete_v = false;

template <typename Type>
constexpr bool is_complete_v<Type, decltype(void(sizeof(Type)))> = true;

template <typename Type, typename = void>
struct HasName : std::false_type {};

template <typename Type>
struct HasName<Type, std::enable_if_t<is_complete_v<Type>,
                                      std::void_t<decltype(Type::Name())>>>
    : std::true_type {};
}  // namespace detail

template <typename ToState>
struct TransitionTo;

/// @brief An input-driven state machine for embedded applications.
template <typename Data, typename InitialState, typename... States>
class StateMachine {
 public:
  using LogCallback = std::function<void(std::string_view, std::string_view)>;

  /// @brief Constructs a new state machine.
  /// @param data           Data shared by every state of the state machine
  /// @param initial_state  The initial state the state machine begins in.
  /// @param ...states      The remaining states the state machine can
  /// transition to.
  explicit StateMachine(Data &data, InitialState initial_state,
                        States... states);

  /// @brief Processes the current state, calling its Process(...) function.
  /// Note: Might result in a transition.
  void Process(Data &data);

  /// @brief Forwards the event to the currently active state.
  /// Note: Might result in a transition.
  template <typename Event>
  void Handle(Data &data, const Event &event);

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
  void InitialTransition(...) {};
  template <typename State>
  auto InitialTransition(State &state, Data &data)
      -> decltype(state.OnEnter(data));

  /// @brief The list of states the statemachine holds, no duplicates possible
  std::tuple<InitialState, States...> states_;

  /// @brief Holds a pointer to the currently active state
  std::variant<InitialState *, States *...> current_state_{
      &std::get<0>(states_)};

  /// @brief The logging callback
  LogCallback log_cb_;
};

/// @brief Defines a transition to a state, moves the statemachine to the state
/// if executed.
/// @tparam ToState     The state the transition targets
template <typename ToState>
struct TransitionTo {
  /// @brief Executes this transition
  template <typename StateMachine, typename FromState, typename Data,
            typename... Event>
  void Execute(StateMachine &machine, FromState &from, Data &data,
               const Event &...event);

 private:
  void Exit(...) {};
  template <typename State, typename Data>
  auto Exit(State &from, Data &data) -> decltype(from.OnExit(data));
  template <typename State, typename Data, typename Event>
  auto Exit(State &from, Data &data, const Event &event)
      -> decltype(from.OnExit(data, event));

  void Enter(...) {};
  template <typename State, typename Data>
  auto Enter(State &to, Data &data) -> decltype(to.OnEnter(data));
  template <typename State, typename Data, typename Event>
  auto Enter(State &to, Data &data, const Event &event)
      -> decltype(to.OnEnter(data, event));

  template <typename StateMachine, typename FromState,
            std::enable_if_t<!detail::HasName<FromState>::value ||
                                 !detail::HasName<ToState>::value,
                             bool> = true>
  void Log(StateMachine & /*machine*/);
  template <typename StateMachine, typename FromState,
            std::enable_if_t<detail::HasName<FromState>::value &&
                                 detail::HasName<ToState>::value,
                             bool> = true>
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

  template <typename StateMachine, typename FromState, typename Data,
            typename... Event>
  void Execute(StateMachine &machine, FromState &from, Data &data,
               const Event &...event);

 private:
  std::variant<Transitions...> transition_;
};

/// @brief Convenience transition that can contain differnt transitions or
/// nothing.
/// @tparam ...Transitions  The transitions that can be contained
template <typename... Transitions>
struct Maybe : public Either<Transitions..., DoNothing> {
  using Either<Transitions..., DoNothing>::Either;
};

// =======================================================================
// Implementation of StateMachine Class
// =======================================================================

template <typename Data, typename InitialState, typename... States>
StateMachine<Data, InitialState, States...>::StateMachine(
    Data &data, InitialState /*initial_state**/, States... /*states*/) {
  auto state_visitor = [this, &data](auto *state) {
    InitialTransition(*state, data);
  };
  std::visit(state_visitor, current_state_);
}

template <typename Data, typename InitialState, typename... States>
void StateMachine<Data, InitialState, States...>::Process(Data &data) {
  auto state_visitor = [this, &data](auto *state) {
    state->Process(data).Execute(*this, *state, data);
  };
  std::visit(state_visitor, current_state_);
}

template <typename Data, typename InitialState, typename... States>
template <typename Event>
void StateMachine<Data, InitialState, States...>::Handle(Data &data,
                                                         const Event &event) {
  auto state_vistor = [this, &data, &event](auto *state) -> void {
    state->Handle(data, event).Execute(*this, *state, data, event);
  };
  std::visit(state_vistor, current_state_);
}

template <typename Data, typename InitialState, typename... States>
void StateMachine<Data, InitialState, States...>::SetLogCallback(
    LogCallback log_cb) {
  log_cb_ = std::move(log_cb);
}

template <typename Data, typename InitialState, typename... States>
template <typename State>
auto StateMachine<Data, InitialState, States...>::TransitionTo() -> State & {
  auto &state = std::get<State>(states_);
  current_state_ = &state;
  return state;
}

template <typename Data, typename InitialState, typename... States>
template <typename State>
auto StateMachine<Data, InitialState, States...>::InitialTransition(
    State &state, Data &data) -> decltype(state.OnEnter(data)) {
  state.OnEnter(data);
}

// =======================================================================
// Implementation of TransitionTo Class
// =======================================================================
template <typename ToState>
template <typename StateMachine, typename FromState, typename Data,
          typename... Event>
void TransitionTo<ToState>::Execute(StateMachine &machine, FromState &from,
                                    Data &data, const Event &...event) {
  Log<StateMachine, FromState>(machine);
  Exit(from, data, event...);
  auto &to = machine.template TransitionTo<ToState>();
  Enter(to, data, event...);
}

template <typename ToState>
template <typename State, typename Data>
auto TransitionTo<ToState>::Exit(State &from, Data &data)
    -> decltype(from.OnExit(data)) {
  from.OnExit(data);
}

template <typename ToState>
template <typename State, typename Data, typename Event>
auto TransitionTo<ToState>::Exit(State &from, Data &data, const Event &event)
    -> decltype(from.OnExit(data, event)) {
  from.OnExit(data, event);
}

template <typename ToState>
template <typename State, typename Data>
auto TransitionTo<ToState>::Enter(State &to, Data &data)
    -> decltype(to.OnEnter(data)) {
  to.OnEnter(data);
}

template <typename ToState>
template <typename State, typename Data, typename Event>
auto TransitionTo<ToState>::Enter(State &to, Data &data, const Event &event)
    -> decltype(to.OnEnter(data, event)) {
  to.OnEnter(data, event);
}

template <typename ToState>
template <typename StateMachine, typename FromState,
          std::enable_if_t<!detail::HasName<FromState>::value ||
                               !detail::HasName<ToState>::value,
                           bool>>
void TransitionTo<ToState>::Log(StateMachine & /*machine*/) {
  static_assert(detail::is_complete_v<FromState>,
                "FromState must be fully defined before use");
  static_assert(detail::is_complete_v<ToState>,
                "ToState must be fully defined before use");
}

template <typename ToState>
template <typename StateMachine, typename FromState,
          std::enable_if_t<detail::HasName<FromState>::value &&
                               detail::HasName<ToState>::value,
                           bool>>
void TransitionTo<ToState>::Log(StateMachine &machine) {
  if (machine.log_cb_) {
    machine.log_cb_(FromState::Name(), ToState::Name());
  }
}

// =======================================================================
// Implementation of Either Class
// =======================================================================

template <typename... Transitions>
template <typename StateMachine, typename FromState, typename Data,
          typename... Event>
void Either<Transitions...>::Execute(StateMachine &machine, FromState &from,
                                     Data &data, const Event &...event) {
  auto transition_visitor = [&machine, &from, &data,
                             &event...](auto &transition) -> void {
    transition.Execute(machine, from, data, event...);
  };
  std::visit(transition_visitor, transition_);
}

}  // namespace vsm

#endif
