#ifndef EXAMPLE_SM_DATA_H_
#define EXAMPLE_SM_DATA_H_

inline constexpr int kInitialFloor = 0;

struct SmData {
  int floor = kInitialFloor;
  int current_floor = 0;
  int dest_floor = 0;
  int direction = 0;
};

#endif
