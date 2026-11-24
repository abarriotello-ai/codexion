*This project has been created as part of the 42 curriculum by abarrio.*

# Codexion

## Description

Codexion is a POSIX-threads concurrency simulation modeled on the classic
Dining Philosophers problem. `number_of_coders` people sit in a circle
around a shared "Quantum Compiler." Between every adjacent pair of coders
sits one USB dongle — as many dongles as coders. Compiling quantum code
requires **two** dongles held simultaneously (one in each hand); after
compiling, a coder debugs, then refactors, then immediately tries to
compile again. If a coder doesn't *start* a new compile within
`time_to_burnout` milliseconds of their last one, they burn out and the
simulation stops.

The goal is to orchestrate this with threads and mutexes so that no one
burns out (under a feasible parameter set and the `edf` scheduler), while
avoiding deadlock and giving every coder fair access to the shared dongles
under either of two arbitration policies: `fifo` (first come, first
served) or `edf` (Earliest Deadline First — whoever is closest to burning
out is served first).

## Instructions

Build and run:

```sh
cd coders
make
./codexion number_of_coders time_to_burnout time_to_compile time_to_debug \
    time_to_refactor number_of_compiles_required dongle_cooldown scheduler
```

All eight arguments are mandatory. Durations are in milliseconds.
`scheduler` must be exactly `fifo` or `edf`. Example:

```sh
./codexion 5 2000 300 150 150 10 30 edf
```

Every state change is logged as `<timestamp_ms> <coder_number> <message>`,
e.g. `402 3 is compiling`. The simulation stops the moment someone burns
out, or once every coder has completed at least
`number_of_compiles_required` compiles — whichever comes first.

`make`, `make clean`, `make fclean`, and `make re` behave as usual. The
build uses `-Wall -Wextra -Werror -pthread` with `cc`, and a no-op `make`
does not relink.

## Blocking cases handled

- **Deadlock avoidance (no hold-and-wait)** — a coder never holds one
  dongle while waiting for the other. `dongles_acquire()`
  (`dongle/dongle_acquire.c`) takes **both** dongles in a single step,
  and only when both are available; otherwise it holds nothing and
  waits. Breaking the *hold-and-wait* Coffman condition makes a deadlock
  impossible on its own, whatever the acquisition order.
- **Fair arbitration (FIFO / EDF)** — every dongle keeps its own
  hand-rolled binary min-heap of the coders requesting it
  (`scheduler/heap*.c`), ordered by a key: the arrival ticket (`fifo`)
  or the requester's burnout deadline `last_compile_start +
  time_to_burnout` (`edf`), with the arrival ticket as tie-breaker. The
  ticket comes from one **global** counter, so a coder has the same key
  in both of its queues and the priority order is consistent around the
  whole table. A dongle goes to the highest-priority requester that can
  actually use it right now: a coder yields a dongle to a
  higher-priority neighbour if that neighbour could take both of its
  dongles now, or (under `edf`) if that neighbour is **urgent** — its
  deadline is closer than `time_to_compile + dongle_cooldown` plus a
  5 ms margin, i.e. taking the dongle now would make the neighbour miss
  its deadline. A coder that cannot compile yet does not block its
  other neighbour, so no dongle sits idle for nothing.
- **Cooldown management** — a released dongle moves to a
  `DONGLE_COOLING_DOWN` state with `cooldown_ready_at = now +
  dongle_cooldown`, and only becomes `DONGLE_AVAILABLE` again once that
  deadline has passed (`dongle_cooldown.c`). The end of a cooldown is the
  one event nobody signals, so a coder waiting on a cooling dongle does a
  `pthread_cond_timedwait` that ends exactly at `cooldown_ready_at`
  (`dongle_wait.c`) — no timer thread and no polling.
- **Waiting without burning CPU** — a waiting coder sleeps on its **own**
  condition variable and is woken only by a change that can affect its
  decision: an enqueue, take or release by a coder at distance 1 or 2
  around the table (its decision reads its own dongles, its neighbours'
  queue entries and each neighbour's other dongle — nothing further away).
  An earlier design polled every 1 ms on one shared condition and
  broadcast to every waiter on each release: with 1000 coders that cost
  ~340% CPU and over 800k context switches in under 3 s, starved the
  monitor, and delayed the burnout log by ~100 ms. Now the same run uses
  ~30% CPU.
- **Precise burnout detection** — a dedicated monitor thread
  (`monitor.c`) computes every live coder's deadline and sleeps until the
  nearest one (capped at 5 ms). Deadlines only move later, so no burnout
  can happen before that instant. On a burnout it raises the stop flag
  *before* queueing for the log mutex, so it never waits behind coders
  that are about to print: measured within the 10 ms budget up to 4000
  coders on 8 cores.
- **Log serialization, ordering and silence after the end** — every line
  is produced under one `log_lock` mutex (`logging/log.c`), so two messages
  never interleave. The timestamp is taken inside that lock, so lines come
  out in time order. A coder takes its timestamp, then checks the stop
  flag, then prints — all inside the lock; the monitor raises the stop
  flag before taking the burnout timestamp. So every printed line predates
  the burnout, and **nothing is printed after "burned out"**.
- **Compile start vs. burnout** — "is compiling" is printed and stored as
  the new `last_compile_start` in one step with one timestamp, under the
  coder's `state_lock` together with a stop-flag check; the monitor checks
  the deadline and raises the stop flag under that same lock. Either the
  coder starts first (and the monitor sees the new deadline) or the monitor
  stops the simulation first (and the coder never prints the line).
- **Stopping promptly** — no coder ever sleeps in `usleep`: each phase
  (compile, debug, refactor) is a `pthread_cond_timedwait` until an
  absolute instant on the coder's own condition variable, which the stop
  broadcast interrupts. A burnout ends the program immediately even if a
  neighbour is in the middle of a 24-day compile, and absolute targets
  mean wake-up latency never accumulates across cycles.
- **Start gate** — every coder waits at a gate that the monitor opens once
  all threads exist, fixing time 0 for everyone at once (creating
  thousands of threads takes tens of ms). If any `pthread_create` fails,
  the gate never opens: the threads already created exit without printing
  anything, and the program reports the error on stderr.
- **A liveness bug found and fixed during testing** — the first version
  acquired the two dongles one after the other (lower index first).
  With feasible parameters such as `3 700 200 100 100 7 0 edf` it burned
  out almost every run. The logs showed the cause: a coder held its
  first dongle, unused, for hundreds of milliseconds while waiting for
  the second, and the urgent coder next to it could not even join the
  second dongle's queue, so the per-dongle EDF queues never saw it.
  Acquiring both dongles at once, with the urgency rule above, took the
  scheduling burnouts on the test set to zero.
- **Host stalls on virtual machines** — on WSL2 the whole VM
  occasionally freezes: a trivial 1 ms `usleep` loop with no threads
  measured a 622 ms stall in 20 s. During such a stall no thread runs
  (the monitor included), so a burnout can appear late. Such a run shows
  a gap in the log longer than any action. (`gettimeofday()` cannot tell
  a VM freeze from a wall-clock jump, and the monotonic
  `clock_gettime` is not authorized.) This does not happen on a native
  machine.
- **`number_of_coders == 1` degenerate case** — with only one coder,
  the subject specifies only one dongle exists: the coder takes it (and
  logs "has taken a dongle"), but a compile needs two distinct dongles,
  so it can never compile. It never calls the two-dongle acquisition
  (which would wait forever for a second dongle) and simply waits for the
  monitor to log its burnout at exactly `time_to_burnout`.

## Thread synchronization mechanisms

- **Per-dongle `pthread_mutex_t`** (`t_dongle.lock`) protects that
  dongle's `state`, `cooldown_ready_at`, `held_by`, and its own
  priority-queue heap. Every read or write of a dongle goes through a
  small helper that takes this lock (`dongle_cooldown.c`,
  `dongle_queue.c`, `dongle_release.c`), so the heap needs no separate
  lock.
- **Arbiter mutex** (`t_sim.arb_lock`) — the decision "can this coder
  take its two dongles?" must look at both of its dongles and at its
  neighbours' dongles. Doing that with per-dongle locks alone would mean
  holding several of them in varying orders. Instead the whole decision,
  the global ticket counter, and the take itself run under `arb_lock`, so
  no two coders can be granted the same dongle.
- **Per-coder `wake` condition variable** (paired with `arb_lock`) — where
  a coder waits for its dongles. After an enqueue, take or release,
  `notify_neighbours()` broadcasts on the `wake` of the coders at distance
  1 and 2 only. `pthread_cond_signal` is not authorized, and it is not
  needed: each coder has its own condition, so a broadcast on
  `coders[j].wake` wakes exactly coder `j`. On every wake the coder
  re-checks the full predicate in a `while` loop; a wakeup never means
  "my turn" by itself. With no cooldown pending the wait is an untimed
  `pthread_cond_wait`; otherwise a `pthread_cond_timedwait` until the
  cooldown ends.
- **Per-coder `sleep_cond` condition variable** (paired with the coder's
  `state_lock`) — the interruptible "sleep" of every phase, of the start
  gate and of the lone coder. One condition per coder, not one shared:
  when a thousand phases end in the same millisecond, each coder
  re-acquires only its own mutex, instead of all of them (and the monitor
  behind them) queueing on one. An earlier version with a shared condition
  measurably delayed the burnout log by 15–36 ms at 1000+ coders.
- **Per-coder `state_lock`** (`t_coder.state_lock`) protects
  `last_compile_start` and `compiles_done` — the fields the monitor reads
  from a coder it doesn't own. The coder publishes its compile start under
  this lock (`log_compile_start()` in `log.c`) and the monitor reads it
  under the same lock (`monitor.c`), so there is no torn read of a
  `long long` timestamp, and the burnout decision and the compile start
  cannot interleave (see above).
- **`stop_lock` + `stop_flag` + `started`** — the simulation-wide state
  every thread checks, only ever read or written under its own mutex
  (`sim_stop.c`, `coder_sleep.c`). `stop_simulation()` raises the flag,
  then broadcasts every coder's `sleep_cond` and `wake`, so no thread can
  keep sleeping after the stop.
- **A single `log_lock`** wraps every `printf` of the simulation,
  guaranteeing that no two messages interleave, that timestamps come out
  in order, and that nothing follows "burned out".
- **Lock order** — `arb_lock` or `log_lock` → `state_lock` → `stop_lock`,
  with a dongle `lock` always a leaf taken under `arb_lock`. `arb_lock` and
  `log_lock` are never held together, and no path takes a lock that comes
  earlier in this order while holding a later one, so the mutexes cannot
  deadlock among themselves. Helgrind and DRD report no data race and no
  lock-order violation.

No global variables are used anywhere — `main` owns a single `t_sim` on
its own stack (its coder/dongle arrays are `malloc`'d, but the `t_sim`
struct itself is not), and every thread (coder or monitor) only ever
reaches shared state through the pointer passed as its
`pthread_create` argument.

## Resources

- Dijkstra's Dining Philosophers problem, and its standard deadlock
  fixes (resource ordering, and acquiring both resources at once).
- The Coffman conditions for deadlock (mutual exclusion, hold-and-wait,
  no preemption, circular wait) as the checklist used to reason about
  which condition each design choice actually breaks.
- Earliest Deadline First (EDF) scheduling as a real-time scheduling
  concept, applied here per dongle with a consistent global key.
- `man pthread_mutex_init`, `man pthread_cond_timedwait`,
  `man gettimeofday` — the POSIX threading primitives this project is
  built on.

### AI usage disclosure

AI was used throughout this project's development, in the way
described by this curriculum's own AI-usage guidance: as a teacher
for explanatory purposes and for making this README.md. AI was also
used to test the simulation, to diagnose the liveness bug described
above, and to implement its fix (the two-dongle acquisition and
urgency rule in `srcs/dongle/`).

In a later hardening pass, AI was used to build an automated output
validator and stress-test battery (from the subject's examples up to
10000 coders and `INT_MAX` durations, repeated soak runs, valgrind,
helgrind and DRD), to measure CPU usage and burnout-log latency before
and after, and to diagnose and fix the bugs that battery exposed: the
missing 42 headers, log lines printed after a burnout, `usleep` phases
that could not be interrupted, the polling/broadcast wait design that
starved the monitor at scale, out-of-order timestamps, the lone coder's
missing "has taken a dongle", silent partial output when thread
creation fails, and unused fields. Every change is explained in the
personal defense guide so that it can be reproduced and justified.
