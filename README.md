*This project has been created as part of the 42 curriculum by abait-el.*

# Codexion

## Description

Codexion is a multithreaded C simulation of coders competing for scarce
resources before their deadlines expire. Each coder must compile, debug, and
refactor in a loop, but compiling requires two USB dongles held simultaneously:
coder *i* takes the dongle on its left and the dongle on its right. The coders
are arranged in a ring, so every dongle is shared by exactly two neighboring
coders.

The program takes a number of timing parameters and a scheduling policy
(`fifo` or `edf`), simulates the run in real time, and prints a serialized log
of each coder's actions. The run ends when every coder has completed the
required number of compiles, or when a coder "burns out" because it failed to
start compiling within `time_to_burnout`.

The main engineering challenge is concurrency: two coders want the same dongle,
every coder needs two dongles at once, and a fair arbitration must keep the
simulation deadlock-free and starvation-free. The solution queues a request in
both dongle heaps before either is granted, so coder threads either receive a
full pair atomically or wait without holding anything.

## Instructions

### Build

```sh
make
```

The Makefile builds the `codexion` binary with `cc` and the flags
`-Wall -Wextra -Werror -pthread`. It provides the usual rules: `all`, `clean`,
`fclean`, and `re`.

### Run

```sh
./codexion number_of_coders time_to_burnout time_to_compile \
           time_to_debug time_to_refactor number_of_compiles_required \
           dongle_cooldown scheduler
```

| Argument | Meaning |
| --- | --- |
| `number_of_coders` | number of coders in the ring |
| `time_to_burnout` | max ms without starting a compile before a coder burns out |
| `time_to_compile` | ms a compile takes |
| `time_to_debug` | ms a debugging phase takes |
| `time_to_refactor` | ms a refactoring phase takes |
| `number_of_compiles_required` | compiles each coder needs to finish |
| `dongle_cooldown` | ms a dongle is unavailable after being released |
| `scheduler` | `fifo` or `edf` |

Output lines follow this format:

```text
timestamp_in_ms coder_id has taken a dongle
timestamp_in_ms coder_id is compiling
timestamp_in_ms coder_id is debugging
timestamp_in_ms coder_id is refactoring
timestamp_in_ms coder_id burned out
```

### Examples

```sh
./codexion 1 800 200 200 200 10 0 fifo
./codexion 5 2000 200 200 200 10 0 fifo
./codexion 5 2000 200 200 200 7 0 edf
./codexion 5 500 200 200 200 10 0 fifo
./codexion 5 3000 200 200 200 10 400 fifo
./codexion 5 3000 200 200 200 10 800 fifo
./codexion 5 3000 200 200 200 10 800 edf
```

The 500 ms case intentionally burns out. The single-coder case also burns out,
because one coder can never own two distinct dongles. The other benchmark runs
are expected to complete.

## Blocking cases handled

### Deadlock prevention (Coffman's conditions)

Coffman identifies four necessary conditions for deadlock: mutual exclusion,
hold-and-wait, no preemption, and circular wait. Codexion breaks **circular
wait** by design.

A coder never acquires one dongle and then waits for the second one. Instead,
it places a request in the heaps of *both* dongles and receives either both
dongles at once (atomically, under the scheduler mutex) or none at all. Because
a thread can never hold one resource while blocked on another, the hold-and-wait
chain that would form a ring is impossible. The per-dongle mutual exclusion
remains, but no cycle can form around it.

### Starvation prevention

The dongle request heaps are priority queues, ordered by arrival order (FIFO)
or by burnout deadline (EDF, with FIFO as the tie-breaker). Before granting a
dongle, the scheduler checks the highest-priority request in that dongle's heap:
a requester is only allowed to take the dongle if it is itself at the head of
the queue, or the higher-priority waiter cannot actually use the pair right now
(one of its dongles is still held or cooling down, or its deadline leaves no
time to wait). This prevents a lower-priority coder from sneaking past an
urgent one and starving it out.

### Cooldown handling

After a coder releases its dongles, each dongle is marked unavailable until
`available_at_ms = now + dongle_cooldown`. The scheduler does not poll: waiting
coders sleep on a condition variable and wake on a real event (a new request, a
release, or the exact moment the earliest cooldown ends).

### Precise burnout detection

The monitor thread watches every coder's `burnout_deadline_ms` and stops the
simulation when a deadline passes. A small timing tolerance (9 ms) is applied at
the boundary so a scheduler/monitor race cannot kill a coder that is waking
exactly when a feasible cooldown ends.

### Log serialization

All state lines are printed under a dedicated log mutex, so interleaved
`printf` calls from concurrent threads cannot corrupt the output. Lines are
suppressed once the simulation stops, so the final log only reflects the run.

## Thread synchronization mechanisms

The implementation uses three synchronization domains, each answering a
different question.

| Primitive | Protects | Role |
| --- | --- | --- |
| `log_mutex` | stdout | Serializes printed lines |
| `scheduler_mutex` + per-dongle `mutex` | dongle state and request heaps | Arbitrates who gets a dongle pair |
| `state_mutex` + `state_changed` / `sleep_condition` | shared flags, start time, deadlines | Coordinates stopping and timing |

### Scheduling (`scheduler_mutex`, `scheduler_changed`)

The scheduler mutex serializes the grant critical section: queueing a request,
checking both dongles, and granting the pair happen atomically, so two coders
cannot both pass the same availability check. `scheduler_changed` is broadcast
when a request is added or a dongle is released, waking waiting coders. Each
dongle additionally has its own mutex protecting `is_held` and
`available_at_ms`, so the scheduler can read cooldown state without holding the
global lock.

### State and timing (`state_mutex`, `state_changed`, `sleep_condition`)

The state mutex protects the simulation-wide flags (`is_stopped`,
`start_released`), the start time, and each coder's burnout deadline. Work-phase
sleeps use a dedicated `sleep_condition` so coders are not woken every time
someone updates a deadline; deadline changes and stop events broadcast on
`state_changed` instead.

### Communication between coders and the monitor

Coders write their burnout deadline under `state_mutex` each time they start a
compile. The monitor reads those same deadlines under the same lock and waits on
`state_changed` until the earliest one, deciding whether the run should end.
When a coder finishes all required compiles it sets `is_finished`, so the
monitor can detect clean completion. Stopping is a broadcast-driven event: all
sleeping coder threads are woken, and the scheduler wakes every waiter that is
blocked on `scheduler_changed`.

## Resources

- E. G. Coffman and P. J. Denning, *Operating Systems Theory* — the formal
  deadlock conditions and how to break them.
- A. Silberschatz, P. Galvin, G. Gagne, *Operating System Concepts* — chapters
  on process synchronization and deadlocks.
- POSIX threads manual pages: `man pthread_create`, `man pthread_cond_timedwait`,
  `man pthread_mutex_lock` — the primitives used throughout this project.
- G. C. Buttazzo, *Hard Real-Time Computing Systems* — earliest-deadline-first
  scheduling, the basis of the `edf` policy.

### How AI was used

AI-assisted tooling was used to improve code structure and readability.
