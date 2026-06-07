# Multithreaded Ludo Simulation Engine (C++)

A concurrent board game simulation developed in C++ that models a complete multiplayer Ludo environment using multithreading, synchronization primitives, and resource-sharing mechanisms.

The project simulates independent player behavior through parallel execution while ensuring correct game-state transitions using mutexes, semaphores, conditional variables, and thread coordination. The system implements complex game rules, collision detection, event handling, player elimination, and winner determination in a synchronized environment.

---
<img width="588" height="636" alt="Screenshot 2026-06-07 at 6 08 54 PM" src="https://github.com/user-attachments/assets/f030727c-c6f2-4cad-bd00-afd7513fb425" />

## Key Features

* Multi-threaded architecture with dedicated player threads
* Master thread for game monitoring and lifecycle management
* Semaphore-based token and resource synchronization
* Mutex-protected shared resources
* Conditional variable coordination for turn ordering
* Randomized player scheduling and turn execution
* Complete implementation of Ludo gameplay mechanics
* Real-time board state updates
* Player elimination and thread cancellation logic
* Winner ranking and leaderboard generation

---

## System Architecture

### Player Threads

* Four independent player threads simulate concurrent participants.
* Each thread manages token movement, dice rolling, and game actions.
* Players compete for shared resources while maintaining synchronization constraints.

### Master Thread

* Tracks player progress and hit records.
* Validates completion conditions.
* Handles player elimination policies.
* Manages thread lifecycle and cancellation.
* Calculates final rankings and winner positions.

### Resource Management

Two critical shared resources are managed:

* Dice Resource
* Ludo Board Resource

Synchronization mechanisms ensure that only one thread can access a shared resource at a given time while preserving correct game sequencing.

---

## Concurrency Concepts Implemented

### Multithreading

* Pthreads-based concurrent execution
* Independent worker threads
* Thread lifecycle management

### Synchronization

* Mutex locks
* Semaphores
* Condition variables
* Thread-safe resource access

### Thread Coordination

* Ordered board access
* Race-condition prevention
* Deadlock avoidance
* Shared-state consistency

### Thread Cancellation

* Automatic removal of inactive players
* Controlled thread termination
* Master-thread-driven lifecycle management

---

## Algorithms & Logic

### Turn Scheduling

* Randomized turn allocation
* Fair execution guarantees
* Bonus turn handling for sixes

### Collision Detection

* Token overlap detection
* Opponent elimination logic
* Safe-zone verification

### Rule Validation Engine

* Home-column eligibility checks
* Block formation detection
* Exact-roll completion validation
* Consecutive six handling

### State Management

* Token lifecycle tracking
* Hit-rate management
* Player progression monitoring

---

## Technical Skills Demonstrated

* C++
* POSIX Threads (Pthreads)
* Semaphores
* Mutexes
* Condition Variables
* Concurrent Programming
* Synchronization
* Resource Scheduling
* Algorithm Design
* Event-Driven Systems
* State Machines
* Software Architecture

---

## Engineering Challenges Solved

* Prevented race conditions on shared game resources.
* Guaranteed correct execution order in a highly concurrent environment.
* Designed a scalable architecture capable of managing multiple interacting threads.
* Implemented complex rule validation while maintaining thread safety.
* Developed robust player elimination and completion workflows.

---

## Relevance to Software Engineering Roles

This project demonstrates skills directly applicable to:

### CAD Development (C++)

* Complex algorithm implementation
* State management
* High-performance C++ programming
* Mathematical and logical problem solving

### Application Development (C++)

* Modular software architecture
* Concurrent application design
* Feature integration
* Resource management

### Mathematics & R&D

* Rule-based algorithm development
* Edge-case handling
* Optimization logic
* Large state-space problem solving

### QA & Engineering Software

* Validation systems
* Event simulation
* Testing complex workflows
* Consistency and correctness verification
