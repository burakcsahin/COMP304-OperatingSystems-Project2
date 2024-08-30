# comp-304-operating-systems-project-2-3bornagain
# ATC Simulator

This project simulates an Air Traffic Control (ATC) system using POSIX threads.

## Structure

- `include/`: Header files
- `src/`: Source files
- `Makefile`: Build script

## Building

To build the project, run:

`make clean - make all`

Execute the compiled binary with the desired arguments. For example, to run with a simulation time of 200 seconds, a landing probability of 0.5, and log snapshots every 20 seconds:

`./bin/atc_simulator -s 200 -p 0.5 -n 20`

# Air Traffic Control Simulator

## Introduction

The purpose of this project is to simulate an air traffic control (ATC) system that manages the take-off and landing of planes using a single runway. The simulator ensures collision-free and deadlock-free operation while preventing starvation of planes either waiting to land or take off. The implementation utilizes POSIX threads (pthreads) to manage multiple planes and the control tower concurrently.

## Objectives

1. **Part I**: Implement a basic ATC simulator with real-time scheduling, favoring landing planes to minimize fuel consumption.
2. **Part II**: Extend the simulator to prevent starvation of planes waiting on the ground by incorporating a maximum wait-time and counter.

## Environment Setup

1. **Directories**:
   - `include/`: Header files
   - `src/`: Source files
   - `obj/`: Object files
   - `bin/`: Executable files

2. **Files**:
   - `include/atc_simulator.h`: Header file for the ATC simulator.
   - `include/pthread_sleep.h`: Header file for the pthread sleep function.
   - `src/atc_simulator.cpp`: Main source file for the ATC simulator.
   - `src/pthread_sleep.cpp`: Source file for the pthread sleep function.
   - `Makefile`: Build automation tool.
   - `README.md`: Project documentation.
   - `planes.log`: Log file for plane activities.

## Implementation

### Part I: Basic ATC Simulator

**Functional Requirements**:
- Single runway for both take-offs and landings.
- Ensure only one plane on the runway at a time.
- Use real-time simulation.
- Planes notify the control tower when ready to land or depart.
- Tower acknowledges the position in the queue and signals the plane in the first position to proceed.
- Planes take 2 seconds (2t) to land or take off.
- Probabilities dictate plane arrivals and departures.
- Favor landing planes until none are waiting, then allow one take-off before favoring landing planes again.
- Initial state includes one plane waiting to take off and one plane waiting to land.

### Part II: Starvation Prevention

**Functional Requirements**:
- Avoid starvation for planes waiting on the ground.
- Control tower favors landing planes unless:
  - No planes are waiting to land, or
  - Five or more planes are waiting to take off.
- Implement a counter for planes waiting on the ground.

## Logs

**Log File Structure**:
- Plane ID (even for landing, odd for departing)
- Status (L for landing, D for departing)
- Request time for runway usage
- Runway usage start time
- Turnaround time (runway start time - request time)
