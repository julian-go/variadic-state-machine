#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"
#include "vsm/vsm.hpp"

TEST_CASE("Process called") {
  struct Data {
    int i{0};
  } data;

  struct State {
    auto Process(Data& data) -> vsm::DoNothing {
      data.i++;
      return {};
    }
  };

  auto sm = vsm::StateMachine(data, State{});

  sm.Process(data);
  sm.Process(data);
  sm.Process(data);

  CHECK(data.i == 3);
}

TEST_CASE("Initial Transition called") {
  struct Data {
    int i{0};
  } data;

  struct StateB;

  struct StateA {
    void OnEnter(Data& data) { data.i++; }

    auto Process(Data& /*data*/) -> vsm::TransitionTo<StateB> {
      return vsm::TransitionTo<StateB>{};
    }
  };

  struct StateB {
    void OnEnter(Data& data) { data.i++; }

    auto Process(Data& /*data*/) -> vsm::TransitionTo<StateA> {
      return vsm::TransitionTo<StateA>{};
    }
  };

  auto sm = vsm::StateMachine(data, StateA{}, StateB{});

  // due to initial transition
  CHECK(data.i == 1);
}

TEST_CASE("Entry called") {
  struct Data {
    int i{0};
  } data;

  struct StateB;

  struct StateA {
    void OnEnter(Data& data) { data.i++; }

    auto Process(Data& /*data*/) -> vsm::TransitionTo<StateB> {
      return vsm::TransitionTo<StateB>{};
    }
  };

  struct StateB {
    void OnEnter(Data& data) { data.i++; }

    auto Process(Data& /*data*/) -> vsm::TransitionTo<StateA> {
      return vsm::TransitionTo<StateA>{};
    }
  };

  auto sm = vsm::StateMachine(data, StateA{}, StateB{});

  // due to initial transition
  CHECK(data.i == 1);

  sm.Process(data);
  sm.Process(data);
  sm.Process(data);

  CHECK(data.i == 4);
}

TEST_CASE("Exit called") {
  struct Data {
    int i{0};
  } data;

  struct StateB;

  struct StateA {
    auto Process(Data& /*data*/) -> vsm::TransitionTo<StateB> {
      return vsm::TransitionTo<StateB>{};
    }

    void OnExit(Data& data) { data.i++; }
  };

  struct StateB {
    auto Process(Data& /*data*/) -> vsm::TransitionTo<StateA> {
      return vsm::TransitionTo<StateA>{};
    }

    void OnExit(Data& data) { data.i++; }
  };

  auto sm = vsm::StateMachine(data, StateA{}, StateB{});

  sm.Process(data);
  sm.Process(data);
  sm.Process(data);

  CHECK(data.i == 3);
}

TEST_CASE("Entry & Exit called") {
  struct Data {
    int a1{0};
    int a2{0};
    int b1{0};
    int b2{0};
  } data;

  struct StateB;

  struct StateA {
    void OnEnter(Data& data) { data.a1++; }

    auto Process(Data& /*data*/) -> vsm::TransitionTo<StateB> {
      return vsm::TransitionTo<StateB>{};
    }

    void OnExit(Data& data) { data.a2++; }
  };

  struct StateB {
    void OnEnter(Data& data) { data.b1++; }

    auto Process(Data& /*data*/) -> vsm::TransitionTo<StateA> {
      return vsm::TransitionTo<StateA>{};
    }

    void OnExit(Data& data) { data.b2++; }
  };

  auto sm = vsm::StateMachine(data, StateA{}, StateB{});

  CHECK(data.a1 == 1);
  CHECK(data.a2 == 0);
  CHECK(data.b1 == 0);
  CHECK(data.b2 == 0);

  sm.Process(data);

  CHECK(data.a1 == 1);
  CHECK(data.a2 == 1);
  CHECK(data.b1 == 1);
  CHECK(data.b2 == 0);

  sm.Process(data);

  CHECK(data.a1 == 2);
  CHECK(data.a2 == 1);
  CHECK(data.b1 == 1);
  CHECK(data.b2 == 1);

  sm.Process(data);

  CHECK(data.a1 == 2);
  CHECK(data.a2 == 2);
  CHECK(data.b1 == 2);
  CHECK(data.b2 == 1);
}

TEST_CASE("Event transition 1") {
  struct Data {
    int i1{0};
    int i2{0};
  } data;

  struct Event1 {};

  struct StateB;

  struct StateA {
    static constexpr std::string_view Name() { return "StateA"; };

    void OnEnter(Data& data, const Event1& /*event*/) {
      std::cout << "transition to A\n";
      data.i1++;
    }

    auto Process(Data& /*data*/) -> vsm::DoNothing { return {}; }

    void OnExit(Data& data, const Event1& /*event*/) {
      data.i2++;
    }

    auto Handle(Data& /*data*/, Event1 /*event*/) -> vsm::TransitionTo<StateB> {
      return {};
    }
  };

  struct StateB {
    static constexpr std::string_view Name() { return "StateB"; };

    void OnEnter(Data& data, const Event1& /*event*/) {
      data.i1++;
    }

    auto Process(Data& /*data*/) -> vsm::DoNothing { return {}; }

    void OnExit(Data& data, const Event1& /*event*/) {
      data.i2++;
    }

    auto Handle(Data& /*data*/, Event1 /*event*/) -> vsm::TransitionTo<StateA> {
      return {};
    }
  };

  auto sm = vsm::StateMachine(data, StateA{}, StateB{});

  sm.SetLogCallback(
      [](auto a, auto b) { std::cout << a << " >> " << b << "\n"; });

  // No initial transition, since OnEnter is only defined for events
  CHECK(data.i1 == 0);
  CHECK(data.i2 == 0);

  sm.Process(data);

  CHECK(data.i1 == 0);
  CHECK(data.i2 == 0);

  sm.Handle(data, Event1{});

  CHECK(data.i1 == 1);
  CHECK(data.i2 == 1);

  sm.Handle(data, Event1{});

  CHECK(data.i1 == 2);
  CHECK(data.i2 == 2);
}
