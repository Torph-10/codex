*This project has been created as part of the 42 curriculum by <abelgarh>.*

# Codexion

## Description

**Codexion** is a multithreading and concurrency project from the 42 curriculum. The goal is to simulate several coders competing for a limited set of shared resources called **dongles**, while ensuring that the simulation remains safe, fair, and responsive.

Each coder is represented by a POSIX thread (`pthread`). Coders repeatedly perform three operations:

1. **Compile** — requires two dongles simultaneously.
2. **Debug** — performed after compiling.
3. **Refactor** — performed after debugging.

After refactoring, the coder tries to compile again until the required number of compilations has been reached.

The number of dongles is equal to the number of coders, with one dongle shared between each pair of neighbouring coders in the circular hub.

The simulation must also respect a burnout deadline. A coder burns out if they do not start a new compilation within `time_to_burnout` milliseconds from the beginning of their previous compilation (or from the beginning of the simulation).

Two scheduling policies are supported:

* **FIFO (First In, First Out):** requests are served according to their arrival order.
* **EDF (Earliest Deadline First):** requests with the earliest burnout deadline are served first.

The project focuses on concurrent programming, thread synchronization, resource management, scheduling, deadlock prevention, and precise timing.

## Instructions

### Compilation

The project is compiled using the provided `Makefile`.

```bash
make
```

To remove object files:

```bash
make clean
```

To remove object files and the executable:

```bash
make fclean
```

To rebuild the project from scratch:

```bash
make re
```

### Execution

The program takes the following arguments:

```text
./codexion number_of_coders time_to_burnout time_to_compile time_to_debug time_to_refactor number_of_compiles_required dongle_cooldown scheduler
```

The arguments are:

| Argument                      | Description                                                                |
| ----------------------------- | -------------------------------------------------------------------------- |
| `number_of_coders`            | Number of coder threads and dongles                                        |
| `time_to_burnout`             | Maximum time, in milliseconds, allowed before a coder must start compiling |
| `time_to_compile`             | Compilation duration in milliseconds                                       |
| `time_to_debug`               | Debugging duration in milliseconds                                         |
| `time_to_refactor`            | Refactoring duration in milliseconds                                       |
| `number_of_compiles_required` | Number of compilations required for each coder                             |
| `dongle_cooldown`             | Cooldown duration of a dongle after it is released                         |
| `scheduler`                   | Scheduling policy: `fifo` or `edf`                                         |

### Examples

Normal execution with FIFO scheduling:

```bash
./codexion 5 2000 200 200 200 10 0 fifo
```

Normal execution with EDF scheduling:

```bash
./codexion 5 2000 200 200 200 10 0 edf
```

A configuration where a coder must burn out because one coder cannot obtain two dongles:

```bash
./codexion 1 800 200 200 200 10 0 fifo
```

A configuration where the complete coder cycle exceeds the burnout deadline:

```bash
./codexion 5 500 200 200 200 10 0 fifo
```
## Logs:
Any state change of a coder must be formatted as follows:

- timestamp_in_ms X has taken a dongle
- timestamp_in_ms X is compiling
- timestamp_in_ms X is debugging
- timestamp_in_ms X is refactoring
- timestamp_in_ms X burned out
X will be the coder number

Rules:

A displayed state message should not be mixed up with another message.
A message announcing that a coder burned out should be displayed no more than 10 ms after the actual burnout.

## Blocking cases handled

The implementation addresses the main concurrency problems that can occur when multiple coder threads compete for shared dongles.

### Deadlock prevention

A deadlock can occur when threads hold resources while waiting for resources held by other threads. In particular, inconsistent dongle acquisition order can create a circular wait.

To prevent this, each coder always acquires dongles in a deterministic order based on their IDs:

```text
lower dongle ID -> higher dongle ID
```

This removes the circular-wait pattern caused by different threads acquiring the same resources in opposite orders.

For example, instead of having:

```text
Coder A: D1 -> D2
Coder B: D2 -> D1
```

both coders follow the same resource ordering rule.

This addresses the circular-wait part of the Coffman deadlock conditions.

### Coffman's conditions

The implementation takes deadlock prevention into account by avoiding the circular-wait condition through ordered resource acquisition.

The relevant Coffman conditions are:

* Mutual exclusion
* Hold and wait
* No preemption
* Circular wait

Dongles are mutually exclusive resources and coders may need to wait while holding a resource, but the deterministic acquisition order prevents a circular wait from being formed.

### Starvation prevention

Waiting coders are stored in a custom priority queue implemented as a binary heap for each dongle.

The scheduler determines the ordering:

* **FIFO:** earlier requests have higher priority.
* **EDF:** earlier deadlines have higher priority.
* `coder_id` is used as a deterministic tie-breaker.

This prevents the ordering from being arbitrary and provides deterministic resource selection.

### Cooldown handling

After a dongle is released, it cannot immediately be reused until its cooldown period has elapsed.

Each dongle stores an `available_at` timestamp indicating when it becomes available again.

A coder therefore has to respect both:

```text
dongle ownership
+
dongle cooldown
```

before acquiring the resource.

### Precise burnout detection

A dedicated **monitor thread** continuously checks the state of the coders.

For every coder that has not completed the required number of compilations, the monitor compares:

```text
current_time - last_compile_start
```

with:

```text
time_to_burnout
```

When the limit is reached, the coder is marked as burned out and the simulation is stopped.

The monitor checks frequently enough to detect burnout within the required timing constraints.

### Log serialization

Multiple coder threads and the monitor thread may print messages concurrently.

To prevent interleaved or corrupted output, logging is protected by a dedicated:

```c
pthread_mutex_t log_mutex;
```

Only one thread can print a protected status message at a time.

### Safe simulation termination

The simulation has a shared `stopped` state protected by:

```c
pthread_mutex_t state_mutex;
```

When either:

* a coder burns out, or
* all coders complete the required number of compilations,

the simulation is marked as stopped.

Waiting threads are notified so that they can leave their waiting state and terminate cleanly.

## Thread synchronization mechanisms

The implementation uses POSIX threads and synchronization primitives to coordinate access to shared resources and communicate between coder threads and the monitor.

### `pthread_mutex_t`

Mutexes are used to protect shared state that must not be modified concurrently.

The implementation uses mutexes for resources such as:

* Dongle ownership and availability
* Simulation stop state
* Log output
* Shared coder state

For example, the state of the simulation is protected with:

```c
pthread_mutex_lock(&sim->state_mutex);
sim->stopped = 1;
pthread_mutex_unlock(&sim->state_mutex);
```

This ensures that multiple threads cannot read and modify the protected state simultaneously in an unsafe way.

### Dongle mutexes

Every dongle has its own mutex:

```c
pthread_mutex_t mutex;
```

This protects information belonging to that dongle, including its availability and waiting state.

This prevents two coder threads from simultaneously treating the same dongle as available.

### `pthread_cond_t`

Each dongle also has a condition variable:

```c
pthread_cond_t cond;
```

Condition variables allow coder threads to wait without continuously consuming CPU while a dongle is unavailable.

A waiting coder sleeps until another thread changes the resource state and signals the waiting threads.

The condition variable is used together with a mutex:

```text
lock mutex
    check condition
    wait if necessary
unlock mutex
```

When the resource may become available, waiting threads are awakened and re-check the condition.

### Custom event / wake-up mechanism

The project also uses a custom wake-up mechanism based on condition-variable broadcasting.

When the simulation stops, all dongle condition variables are notified so that blocked coder threads can wake up and check the shared `stopped` state.

Conceptually:

```text
monitor detects burnout
        ↓
stopped = 1
        ↓
wake up waiting coders
        ↓
coders re-check stopped
        ↓
coders terminate safely
```

This prevents threads from remaining blocked indefinitely after the simulation has already ended.

### Race condition prevention

A race condition can happen when several threads access the same shared variable at the same time and at least one of them modifies it.

For example, the simulation stop flag is shared by all coder threads and the monitor.

Instead of doing an unsafe access:

```c
if (sim->stopped)
```

the implementation accesses the value while holding `state_mutex`, copies it locally, and then releases the mutex.

This guarantees a consistent read.

The same principle is used when updating shared state such as:

```text
stopped
last_compile_start
compile_count
dongle ownership
dongle availability
```

### Communication between coders and the monitor

The coders update their state when important events occur, such as starting a compilation.

The monitor reads that state while holding the appropriate mutex.

The communication therefore follows:

```text
Coder thread
    ↓
updates shared state
    ↓
state_mutex
    ↓
Monitor thread
    ↓
reads shared state
    ↓
checks burnout / completion
```

When the monitor detects a terminal condition, it updates the shared stop state and wakes waiting coder threads through the condition variables.

## Scheduling

### FIFO

FIFO stands for **First In, First Out**.

Each request receives an arrival number when it joins a dongle's waiting queue.

Example:

```text
Coder 2 -> arrival order 1
Coder 4 -> arrival order 2
Coder 1 -> arrival order 3
```

The scheduler selects Coder 2 first, then Coder 4, then Coder 1.

### EDF

EDF stands for **Earliest Deadline First**.

Each request receives a deadline based on:

```text
last_compile_start + time_to_burnout
```

The request with the earliest deadline gets the highest priority.

Example:

```text
Coder 1 -> deadline 1200
Coder 2 -> deadline 900
Coder 3 -> deadline 1100
```

The order is:

```text
Coder 2
Coder 3
Coder 1
```

If two requests have the same scheduling value, `coder_id` is used as a deterministic tie-breaker.

## Data structures

### Custom binary heap

The waiting queue for each dongle is implemented as a custom binary heap.

The project does not rely on a standard-library priority queue.

The heap supports operations such as:

```text
heap_push()
heap_top()
heap_remove()
```

The heap uses the scheduler's priority rule to maintain the correct request at the top.

### Requests

Each waiting request contains information such as:

```text
coder_id
arrival_order
deadline
```

This allows the same queue implementation to support both FIFO and EDF scheduling.

## Project structure

The source code is organized around the main responsibilities of the simulation, including:

```text
Parsing
    ↓
Configuration
    ↓
Simulation initialization
    ↓
Coder threads
    ↓
Dongle management
    ↓
Custom scheduling / heap
    ↓
Monitor thread
    ↓
Simulation termination
    ↓
Cleanup
```

## Resources

### POSIX Threads

* https://dev.to/yel-bakk/codexion-4fk8

### Concurrency and deadlocks

* https://www.youtube.com/watch?v=d9s_d28yJq0&list=PLfqABt5AS4FmuQf70psXrsMLEDQXNkLq2

### Scheduling

* Wikipedia — Earliest deadline first scheduling:
  https://en.wikipedia.org/wiki/Earliest_deadline_first_scheduling
* Wikipedia — FIFO scheduling:
  https://en.wikipedia.org/wiki/FIFO_(computing_and_electronics)

### Data structures

* Binary heap:
  https://www.youtube.com/watch?v=HqPJF2L5h9U&t=1850s

### AI usage

AI tools were used as a learning and development aid during the project.

They were used for:

* Understanding POSIX threads, mutexes, condition variables, race conditions, deadlocks, and thread scheduling.
* Explaining the relationship between processes, threads, stacks, shared address space, and CPU execution.
* Reviewing and explaining the implementation of the custom binary heap and scheduling logic.
* Helping reason about FIFO and EDF scheduling and request prioritization.
* Assisting with documentation and README preparation.

AI-generated suggestions were reviewed, tested, and adapted to match the project requirements and the final implementation. The final code and design decisions remain the responsibility of the project authors.

## Constraints

The project follows the required Codexion constraints, including:

* No global variables.
* POSIX threads for coder execution.
* Mutex-based protection of shared resources.
* Condition variables for waiting and wake-up.
* FIFO and EDF scheduling.
* A custom priority queue implementation.
* A dedicated monitor thread for burnout detection.
* Proper cleanup and thread termination.
