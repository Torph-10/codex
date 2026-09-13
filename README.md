*This project has been created as part of the 42 curriculum by <abelgarh>.*

# Codexion

## Description

**Codexion** is a multithreading and concurrency project from the 42 curriculum. The goal is to simulate several coders competing for a limited set of shared resources called **dongles**, while ensuring that the simulation remains safe, fair, and responsive.

Each coder is represented by a POSIX thread (`pthread`). Coders repeatedly perform three operations:

1. **Compile** — requires two dongles simultaneously.
2. **Debug** — performed after compiling.
3. **Refactor** — performed after debugging.

After refactoring, the coder tries to compile again until the required number of compilations has been reached.

The number of dongles is equal to the number of coders, with one dongle shared between each pair of neighbouring coders in the circular hub. Because of this ring topology, **each dongle can only ever be contested by exactly two coders** — its left neighbour and its right neighbour. This is an important structural fact: it means each dongle's waiting queue never holds more than two requests at a time.

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

A configuration with a large cooldown, forcing coders to queue for dongles (good for observing FIFO vs EDF grant-order differences):

```bash
./codexion 5 3000 200 200 200 10 800 fifo
./codexion 5 3000 200 200 200 10 800 edf
```

## Logs

Any state change of a coder must be formatted as follows:

- `timestamp_in_ms X has taken a dongle`
- `timestamp_in_ms X is compiling`
- `timestamp_in_ms X is debugging`
- `timestamp_in_ms X is refactoring`
- `timestamp_in_ms X burned out`

`X` is the coder number.

Rules:

- A displayed state message should not be mixed up with another message.
- A message announcing that a coder burned out should be displayed no more than 10 ms after the actual burnout.

## Blocking cases handled

The implementation addresses the main concurrency problems that can occur when multiple coder threads compete for shared dongles.

### Deadlock prevention

A deadlock can occur when threads hold resources while waiting for resources held by other threads. In particular, inconsistent dongle acquisition order can create a circular wait.

To prevent this, each coder always acquires dongles in a deterministic order based on their IDs:

```text
lower dongle ID -> higher dongle ID
```

This removes the circular-wait pattern caused by different threads acquiring the same resources in opposite orders. For example, instead of:

```text
Coder A: D1 -> D2
Coder B: D2 -> D1
```

both coders follow the same resource ordering rule. This addresses the circular-wait part of the Coffman deadlock conditions.

### Coffman's conditions

The relevant Coffman conditions are:

* Mutual exclusion
* Hold and wait
* No preemption
* Circular wait

Dongles are mutually exclusive resources and coders may need to wait while holding a resource, but the deterministic acquisition order prevents a circular wait from being formed.

### Starvation prevention

Waiting coders are stored in a custom priority queue implemented as heap for each dongle. Because of the ring topology, **each dongle's waiting queue holds at most two requests** (its two neighbouring coders) — the heap implementation is intentionally specialised for this two-entry maximum.

The scheduler determines the ordering:

* **FIFO:** earlier requests have higher priority.
* **EDF:** earlier deadlines have higher priority.
* `coder_id` is used as a deterministic tie-breaker.

Each queue entry is removed by matching its specific `coder_id`, not by position, so a coder cancelling its own request (e.g. because the simulation stopped while it was still waiting) never removes the other coder's request by mistake.

### Cooldown handling

After a dongle is released, it cannot immediately be reused until its cooldown period has elapsed. Each dongle stores an `available_at` timestamp indicating when it becomes available again. A coder therefore has to respect both:

```text
dongle ownership
+
dongle cooldown
```

before acquiring the resource. While waiting specifically for cooldown to elapse (as opposed to waiting for the dongle to free up), the coder retries on a short interval rather than blocking indefinitely, since no other thread will signal it the instant the cooldown timestamp passes.

### Precise burnout detection

A dedicated **monitor thread** continuously checks the state of the coders. For every coder that has not completed the required number of compilations, the monitor compares:

```text
current_time - last_compile_start
```

with:

```text
time_to_burnout
```

When the limit is reached, the coder is marked as burned out and the simulation is stopped. The monitor checks frequently enough to detect burnout within the required timing constraints.

### Log serialization

Multiple coder threads and the monitor thread may print messages concurrently. To prevent interleaved or corrupted output, logging is protected by a dedicated:

```c
pthread_mutex_t log_mutex;
```

Only one thread can print a protected status message at a time.

### Safe simulation termination

The simulation has a shared `stopped` state protected by:

```c
pthread_mutex_t state_mutex;
```

When either a coder burns out, or all coders complete the required number of compilations, the simulation is marked as stopped. Waiting threads are notified so that they can leave their waiting state, remove their own pending request from any dongle queue they were still waiting on, and terminate cleanly.

## Thread synchronization mechanisms

### `pthread_mutex_t`

Mutexes protect shared state that must not be modified concurrently:

* Dongle ownership, availability, and waiting queue
* Simulation stop state
* Log output
* Shared coder state (`compile_count`, `last_compile_start`)

Example:

```c
pthread_mutex_lock(&sim->state_mutex);
sim->stopped = 1;
pthread_mutex_unlock(&sim->state_mutex);
```

### Dongle mutexes

Every dongle has its own mutex:

```c
pthread_mutex_t mutex;
```

This protects that dongle's ownership, availability, and waiting-queue state, preventing two coder threads from simultaneously treating the same dongle as available or corrupting its queue.

### `pthread_cond_t`

Each dongle also has a condition variable:

```c
pthread_cond_t cond;
```

Condition variables let a coder thread sleep without consuming CPU while it is not yet the highest-priority waiter for a dongle. It is woken either when the dongle is released (broadcast) or when the simulation stops.

```text
lock mutex
    check condition
    wait if necessary
unlock mutex
```

### Custom wake-up mechanism

When the simulation stops, every dongle's condition variable is broadcast so that any blocked coder thread wakes up, re-checks the shared `stopped` state, cleans up its own pending queue entry if it has one, and exits.

```text
monitor detects burnout / completion
        ↓
stopped = 1
        ↓
wake up waiting coders
        ↓
coders re-check stopped
        ↓
coders remove their own queue entry (if any) and terminate
```

### Race condition prevention

Instead of an unsafe direct read:

```c
if (sim->stopped)
```

the implementation accesses shared values while holding the appropriate mutex, copies them locally, then releases the mutex — guaranteeing a consistent read. The same principle applies to `last_compile_start`, `compile_count`, dongle ownership, and dongle availability.

### Communication between coders and the monitor

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

## Scheduling

### FIFO

Each request receives an arrival number when it joins a dongle's waiting queue. The request with the lowest arrival number is served first.

### EDF

Each request receives a deadline based on:

```text
last_compile_start + time_to_burnout
```

The request with the earliest deadline is served first. If two requests have the same deadline, `coder_id` is used as a deterministic tie-breaker.

## Data structures

### Custom priority queue

The waiting queue for each dongle is implemented as a small custom array-based priority queue , since the ring topology guarantees at most two simultaneous waiters per dongle:

```text
heap_push()    — inserts a request, keeping the highest-priority one at index 0
heap_top()     — returns the coder_id of the highest-priority waiter, or -1 if empty
heap_remove()  — removes a request by coder_id
```

`heap_priority()` decides ordering using the scheduler's rule (FIFO arrival order or EDF deadline), with `coder_id` as the final tie-breaker.

### Requests

Each waiting request contains:

```text
coder_id
arrival_order
deadline
```

This allows the same queue implementation to support both FIFO and EDF scheduling.

## Project structure

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
Custom scheduling / priority queue
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

* Wikipedia — Earliest deadline first scheduling: https://en.wikipedia.org/wiki/Earliest_deadline_first_scheduling
* Wikipedia — FIFO scheduling: https://en.wikipedia.org/wiki/FIFO_(computing_and_electronics)


### AI usage

AI tools were used as a learning and development aid during the project. They were used for:

* Understanding POSIX threads, mutexes, condition variables, race conditions, deadlocks, and thread scheduling.
* Explaining the relationship between processes, threads, stacks, shared address space, and CPU execution.
* Reviewing and explaining the implementation of the custom priority queue and scheduling logic, including a correctness review of dongle-queue removal (matching by `coder_id` rather than queue position) after a busy-loop-vs-condition-wait comparison surfaced a related timing issue during testing.
* Helping reason about FIFO and EDF scheduling and request prioritization.
* Assisting with documentation and README preparation.

AI-generated suggestions were reviewed, tested, and adapted to match the project requirements and the final implementation. The final code and design decisions remain the responsibility of the project authors.

## Constraints

* No global variables.
* POSIX threads for coder execution.
* Mutex-based protection of shared resources.
* Condition variables for waiting and wake-up.
* FIFO and EDF scheduling.
* A custom priority queue implementation.
* A dedicated monitor thread for burnout detection.
* Proper cleanup and thread termination.
