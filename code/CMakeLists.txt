cmake_minimum_required(VERSION 3.10)
project(Elevator)

set(CMAKE_CXX_STANDARD 17)

add_executable(elevator  main.cpp Control.cpp Passenger.h Queue.h Elevator.cpp FloorButtons.h)