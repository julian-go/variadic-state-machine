#ifndef VARIANTSTATEMACHINE_VSM_H_
#define VARIANTSTATEMACHINE_VSM_H_

#include <functional>
#include <string_view>
#include <tuple>
#include <variant>

namespace vsm {

template <typename ToState>
struct TransitionTo;
struct DoNothing;
template <typename... Actions>
struct Either;
template <typename... Actions>
struct Maybe;

struct LogInfo {
  std::string_view from_state;
  std::string_view to_state;
  std::string_view event;
};

///@brief The main state machine class
///
/// The state machine uses variadic templates and std::variant to allow for
/// compile-time checking of the states and events.
///
///@tparam InitialState
///@tparam States
template <typename InitialState, typename... States>
class StateMachine {
 public:
  explicit StateMachine(InitialState initial_state, States... states)
      : states_{std::move(initial_state), std::move(states)...} {
    std::visit([this](auto* state) { InitialTransition(*state); },
               current_state_);
  }

  void Process() {
    std::visit([this](auto* state) { DoProcess(state); }, current_state_);
  }

  template <typename SmData>
  void Process(SmData& data) {
    std::visit([this, &data](auto* state) { DoProcess(state, data); },
               current_state_);
  }

  template <typename Event>
  void Handle(const Event& event) {
    auto state_visitor = [this, &event](auto* state) -> void {
      state->Handle(event).Execute(*this, *state, event);
    };
    std::visit(state_visitor, current_state_);
  }

  ///@brief Alternative Handle function that allows for shared data to be
  /// passed
  /// to the states for handling
  ///
  ///@tparam Event The type of event to handle
  ///@tparam SmData The type of shared data to pass to the states
  ///@param event The event to handle
  ///@param data The shared data to pass to the states
  template <typename Event, typename SmData>
  void Handle(const Event& event, SmData& data) {
    auto state_vistor = [this, &event, &data](auto* state) -> void {
      state->Handle(event, data).Execute(*this, *state, event, data);
    };
    std::visit(state_vistor, current_state_);
  }

  void SetLogCallback(std::function<void(LogInfo)> log_cb) {
    log_cb_ = std::move(log_cb);
  }

 private:
  template <typename ToState>
  friend struct TransitionTo;

  template <typename State>
  auto DoTransitionTo() -> State& {
    auto& state = std::get<State>(states_);
    current_state_ = &state;
    return state;
  }

  template <typename Type, typename = void>
  struct HasName : std::false_type {};

  template <typename Type>
  struct HasName<Type, std::void_t<decltype(Type::Name())>> : std::true_type {};

  template <
      typename FromState, typename ToState, typename Event,
      std::enable_if_t<!HasName<FromState>::value || !HasName<ToState>::value ||
                           !HasName<Event>::value,
                       bool> = true>
  void LogTransition() {
    if (log_cb_) {
      log_cb_({"unknown", "unknown", "unknown"});
    }
  }
  template <
      typename FromState, typename ToState, typename Event,
      std::enable_if_t<HasName<FromState>::value && HasName<ToState>::value &&
                           HasName<Event>::value,
                       bool> = true>
  void LogTransition() {
    if (log_cb_) {
      log_cb_({FromState::Name(), ToState::Name(), Event::Name()});
    }
  }

  void InitialTransition(...) {}
  template <typename State>
  auto InitialTransition(State& state) -> decltype(state.OnEnter()) {
    state.OnEnter();
  }

  void DoProcess(...) {};
  template <typename State>
  auto DoProcess(State* state) -> decltype(state->Process()) {
    state->Process();
  }
  template <typename State, typename SmData>
  auto DoProcess(State* state, SmData& data) -> decltype(state->Process(data)) {
    state->Process(data);
  }

  std::tuple<InitialState, States...> states_;
  std::variant<InitialState*, States*...> current_state_{&std::get<0>(states_)};
  std::function<void(LogInfo)> log_cb_;
};

///@brief Standard action to transition to a state
///
///@tparam State The state to transition to
template <typename ToState>
struct TransitionTo {
  template <typename StateMachine, typename FromState, typename Event>
  void Execute(StateMachine& machine, FromState& from_state,
               const Event& event) {
    machine.template LogTransition<FromState, ToState, Event>();
    Exit(from_state, event);
    auto& to_state = machine.template DoTransitionTo<ToState>();
    Enter(to_state, event);
  }

  template <typename StateMachine, typename FromState, typename Event,
            typename SmData>
  void Execute(StateMachine& machine, FromState& from_state, const Event& event,
               SmData& data) {
    machine.template LogTransition<FromState, ToState, Event>();
    Exit(from_state, event, data);
    auto& to_state = machine.template DoTransitionTo<ToState>();
    Enter(to_state, event, data);
  }

 private:
  template <typename State, typename Event>
  auto Exit(State& from_state,
            const Event& event) -> decltype(from_state.OnExit(event)) {
    from_state.OnExit(event);
  }

  template <typename State, typename Event, typename SmData>
  auto Exit(State& from_state, const Event& event,
            SmData& data) -> decltype(from_state.OnExit(event, data)) {
    from_state.OnExit(event, data);
  }

  void Exit(...) {};

  template <typename State, typename Event>
  auto Enter(State& to_state,
             const Event& event) -> decltype(to_state.OnEnter(event)) {
    to_state.OnEnter(event);
  }

  template <typename State, typename Event, typename SmData>
  auto Enter(State& to_state, const Event& event,
             SmData& data) -> decltype(to_state.OnEnter(event, data)) {
    to_state.OnEnter(event, data);
  }

  void Enter(...) {};
};

///@brief Standard action that does nothing
struct DoNothing {
  void Execute(...) const {}
};

///@brief Wrapper to allow for different transitions to be performed by a
/// Handle() call
///
///@tparam Actions List of actions that can be performed
template <typename... Actions>
struct Either {
  template <typename Action>
  // NOLINTNEXTLINE(google-explicit-constructor)
  Either(Action action) : action_{std::move(action)} {}

  template <typename StateMachine, typename FromState, typename Event>
  void Execute(StateMachine& machine, FromState& from_state,
               const Event& event) {
    auto action_visitor = [&machine, &from_state,
                           &event](auto& action) -> void {
      action.Execute(machine, from_state, event);
    };
    std::visit(action_visitor, action_);
  }

  template <typename StateMachine, typename FromState, typename Event,
            typename SmData>
  void Execute(StateMachine& machine, FromState& from_state, const Event& event,
               SmData& data) {
    auto action_visitor = [&machine, &from_state, &event,
                           &data](auto& action) -> void {
      action.Execute(machine, from_state, event, data);
    };
    std::visit(action_visitor, action_);
  }

 private:
  std::variant<Actions...> action_;
};

///@brief Convenience wrapper for OneOf that allows for some actions to be
/// performed (or nothing)
///
///@tparam Actions Actions that might be performed
template <typename... Actions>
struct Maybe : public Either<Actions..., DoNothing> {
  using Either<Actions..., DoNothing>::Either;
};

}  // namespace vsm

#endif
