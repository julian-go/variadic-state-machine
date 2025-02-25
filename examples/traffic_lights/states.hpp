#ifndef TRAFFICLIGHTS_STATES_HPP_
#define TRAFFICLIGHTS_STATES_HPP_

#include <iostream>

#include "vsm/vsm.hpp"

//////////////////////////////////
//////////// Events //////////////
//////////////////////////////////

struct Ambulance {};

struct ButtonPushed {};

//////////////////////////////////
//////////// Data   //////////////
//////////////////////////////////

struct Data {
  int timer = 0;
  bool to_green = true;
};

//////////////////////////////////
//////////// States //////////////
//////////////////////////////////

struct Yellow;
struct Green;

struct Red {
  explicit Red(Data& data) : data_{data} {}

  void OnEnter();
  auto Process() -> vsm::Maybe<vsm::TransitionTo<Yellow>>;
  void OnExit();

  auto Handle(const ButtonPushed& event) -> vsm::TransitionTo<Yellow>;
  auto Handle(const Ambulance&) -> vsm::DoNothing;

  Data& data_;
};

struct Yellow {
  explicit Yellow(Data& data) : data_{data} {}

  void OnEnter();
  auto Process()
      -> vsm::Maybe<vsm::TransitionTo<Red>, vsm::TransitionTo<Green>>;
  void OnExit();

  auto Handle(const ButtonPushed& event) -> vsm::DoNothing;
  auto Handle(const Ambulance&) -> vsm::DoNothing;

  Data& data_;
};

struct Green {
  explicit Green(Data& data) : data_{data} {}

  void OnEnter();
  auto Process() -> vsm::Maybe<vsm::TransitionTo<Yellow>>;
  void OnExit();

  auto Handle(const ButtonPushed& event) -> vsm::DoNothing;
  auto Handle(const Ambulance&) -> vsm::TransitionTo<Yellow>;

  Data& data_;
};

#endif