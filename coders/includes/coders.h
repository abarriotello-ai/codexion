/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   coders.h                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abarrio <abarrio@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/23 20:53:50 by abarrio           #+#    #+#             */
/*   Updated: 2026/10/05 19:14:08 by abarrio          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CODERS_H
# define CODERS_H

# include <pthread.h>
# include <sys/time.h>
# include <stdio.h>
# include <stdlib.h>
# include <string.h>
# include <unistd.h>
# include <limits.h>

// The monitor never sleeps longer than this between two scans, so a
// burnout is detected within a few ms even when no deadline is close.
# define POLL_CAP_MS 5
// EDF only: safety margin added when deciding whether a coder must
// leave a free dongle to a neighbour that is about to burn out.
# define URGENCY_MARGIN_MS 5
// A dongle is shared by exactly two coders: its queue never holds more.
# define WAITQ_CAPACITY 2
// Return values of dongles_acquire.
# define ACQUIRE_OK 0
# define ACQUIRE_ABORTED -1
// Upper bounds accepted by the parser.
# define MAX_CODERS 10000
# define MAX_MS INT_MAX

typedef struct timespec	t_timespec;

// Life cycle of a dongle:
//   AVAILABLE -> HELD (taken by a coder) -> COOLING_DOWN (released,
//   unusable until cooldown_ready_at) -> AVAILABLE.
// The last step is lazy: dongle_available() performs it the first time
// the dongle is checked after its cooldown has elapsed.
typedef enum e_dongle_state
{
	DONGLE_AVAILABLE,
	DONGLE_HELD,
	DONGLE_COOLING_DOWN
}	t_dongle_state;

// Policy deciding who gets a dongle first when both neighbours want it:
//   FIFO: first come, first served (key = arrival ticket).
//   EDF : Earliest Deadline First (key = instant the coder burns out).
typedef enum e_scheduler
{
	SCHED_MODE_FIFO,
	SCHED_MODE_EDF
}	t_scheduler;

// One coder waiting in a dongle's queue. Entries are ordered by key
// (ticket in FIFO, burnout deadline in EDF); arrival_seq breaks ties so
// equal keys are served in arrival order.
typedef struct s_heap_entry
{
	int			coder_idx;
	long long	key;
	long long	arrival_seq;
}	t_heap_entry;

// Binary min-heap used as the priority wait queue of a dongle:
// items[0] is always the coder entitled to the dongle next.
typedef struct s_heap
{
	t_heap_entry	*items;
	int				size;
	int				capacity;
}	t_heap;

// Command line arguments once validated. Every time is in ms.
typedef struct s_config
{
	int			n_coders;
	long long	t_burnout;
	long long	t_compile;
	long long	t_debug;
	long long	t_refactor;
	int			compiles_required;
	long long	dongle_cooldown;
	t_scheduler	scheduler;
}	t_config;

// A dongle lies between two neighbouring coders. `lock` protects every
// other field. waitq holds the coders (at most two) waiting for it.
typedef struct s_dongle
{
	pthread_mutex_t	lock;
	t_dongle_state	state;
	long long		cooldown_ready_at;
	t_heap			waitq;
}	t_dongle;

typedef struct s_sim	t_sim;

// One coder = one thread.
//   dongle_lo / dongle_hi: its two dongles sorted by index, so every
//     coder always handles its pair in the same global order.
//   wake: paired with sim->arb_lock. The coder waits on it inside
//     dongles_acquire until a neighbour changes a dongle or a queue.
//   sleep_cond: paired with state_lock. Used for the start gate, the
//     timed phases (compile, debug, refactor) and to be woken on stop.
//   state_lock: protects last_compile_start and compiles_done, the two
//     fields the monitor reads.
//   last_compile_start: -1 until the first compile. The burnout
//     deadline is last_compile_start (or sim start) + t_burnout.
typedef struct s_coder
{
	int				idx;
	pthread_t		thread;
	t_dongle		*dongle_lo;
	t_dongle		*dongle_hi;
	pthread_cond_t	wake;
	pthread_cond_t	sleep_cond;
	pthread_mutex_t	state_lock;
	long long		last_compile_start;
	int				compiles_done;
	t_sim			*sim;
}	t_coder;

// State shared by every thread.
//   stop_lock: protects started and stop_flag.
//   started: set when the monitor opens the start gate.
//   stop_flag: once set every thread finishes as soon as it notices.
//   log_lock: serialises printing, so lines never mix and stay sorted.
//   arb_lock: arbitration lock. Held while enqueueing, taking or
//     releasing dongles, so 'can I take both?' is one atomic decision.
//     It also protects next_ticket.
//   next_ticket: global arrival counter (FIFO order and tie-breaker).
//   sim_start_ms: time 0 of the logs, set when the gate opens.
struct s_sim
{
	t_config		cfg;
	t_coder			*coders;
	t_dongle		*dongles;
	pthread_mutex_t	stop_lock;
	int				started;
	int				stop_flag;
	pthread_mutex_t	log_lock;
	pthread_mutex_t	arb_lock;
	long long		next_ticket;
	long long		sim_start_ms;
	pthread_t		monitor_thread;
};

// parsing
int			parse_args(int argc, char **argv, t_config *cfg);
int			parse_number(const char *s, long long *out, long long max);
int			parse_scheduler(const char *s, t_scheduler *out);

// structures
int			sim_init(t_sim *sim, t_config *cfg);
void		sim_destroy(t_sim *sim);
void		sim_destroy_locks(t_sim *sim);
int			coder_init(t_sim *sim);
void		destroy_coder_sync(t_coder *c);

// dongle
int			dongles_acquire(t_coder *c);
void		dongles_release(t_coder *c);
int			dongle_available(t_dongle *d);
void		dongle_enqueue(t_dongle *d, int coder_idx, long long key,
				long long ticket);
void		dongle_dequeue(t_dongle *d, int coder_idx);
int			dongle_head(t_dongle *d, t_heap_entry *out);
void		dongle_take(t_dongle *d, int coder_idx);
long long	dongle_cooling_until(t_dongle *d);
void		notify_neighbours(t_coder *c);
void		wait_for_dongles(t_coder *c);

// scheduler (heap)
void		heap_swap(t_heap_entry *a, t_heap_entry *b);
int			heap_less(t_heap_entry *a, t_heap_entry *b);
void		heap_sift_up(t_heap *h, int idx);
void		heap_sift_down(t_heap *h, int idx);
void		heap_push(t_heap *h, int coder_idx, long long key,
				long long seq);
void		heap_remove(t_heap *h, int coder_idx);
long long	compute_key(t_coder *c, long long ticket);

// coder
void		*coder_routine(void *arg);
int			coder_do_compile(t_coder *c);
int			coder_finish_compile(t_coder *c);
int			coder_debug_refactor(t_coder *c);

// monitor
void		*monitor_routine(void *arg);

// logging
void		log_line(t_sim *sim, int coder_idx, const char *msg);
void		log_burnout(t_sim *sim, int coder_idx);
long long	log_compile_start(t_coder *c);

// utils
long long	now_ms(void);
t_timespec	ms_to_timespec(long long ms);
int			stop_requested(t_sim *sim);
void		set_stop(t_sim *sim);
void		stop_simulation(t_sim *sim);
void		wake_dongle_waiters(t_sim *sim);
void		wait_until_stop(t_coder *c);
void		wake_sleepers(t_sim *sim);
void		open_start_gate(t_sim *sim);
int			wait_start_gate(t_coder *c);
int			coder_sleep_until(t_coder *c, long long target_ms);
long long	get_last_compile_start(t_coder *c);

// deadline (scheduler)
long long	coder_deadline(t_sim *sim, t_coder *c);

#endif
