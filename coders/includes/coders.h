/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   coders.h                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abarrio <abarrio@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/23 20:53:50 by abarrio           #+#    #+#             */
/*   Updated: 2026/09/28 11:57:14 by abarrio          ###   ########.fr       */
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

# define POLL_CAP_MS 5
# define URGENCY_MARGIN_MS 5
# define WAITQ_CAPACITY 2
# define ACQUIRE_OK 0
# define ACQUIRE_ABORTED -1
# define MAX_CODERS 10000
# define MAX_MS INT_MAX

typedef struct timespec	t_timespec;

typedef enum e_dongle_state
{
	DONGLE_AVAILABLE,
	DONGLE_HELD,
	DONGLE_COOLING_DOWN
}	t_dongle_state;

typedef enum e_scheduler
{
	SCHED_MODE_FIFO,
	SCHED_MODE_EDF
}	t_scheduler;

typedef struct s_heap_entry
{
	int			coder_idx;
	long long	key;
	long long	arrival_seq;
}	t_heap_entry;

typedef struct s_heap
{
	t_heap_entry	*items;
	int				size;
	int				capacity;
}	t_heap;

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

typedef struct s_dongle
{
	pthread_mutex_t	lock;
	t_dongle_state	state;
	long long		cooldown_ready_at;
	t_heap			waitq;
}	t_dongle;

typedef struct s_sim	t_sim;

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

/* parsing */
int			parse_args(int argc, char **argv, t_config *cfg);
int			parse_number(const char *s, long long *out, long long max);
int			parse_scheduler(const char *s, t_scheduler *out);

/* structures */
int			sim_init(t_sim *sim, t_config *cfg);
void		sim_destroy(t_sim *sim);
void		sim_destroy_locks(t_sim *sim);
int			coder_init(t_sim *sim);
void		destroy_coder_sync(t_coder *c);

/* dongle */
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

/* scheduler (heap) */
void		heap_swap(t_heap_entry *a, t_heap_entry *b);
int			heap_less(t_heap_entry *a, t_heap_entry *b);
void		heap_sift_up(t_heap *h, int idx);
void		heap_sift_down(t_heap *h, int idx);
void		heap_push(t_heap *h, int coder_idx, long long key,
				long long seq);
void		heap_remove(t_heap *h, int coder_idx);
long long	compute_key(t_coder *c, long long ticket);

/* coder */
void		*coder_routine(void *arg);
int			coder_do_compile(t_coder *c);
int			coder_finish_compile(t_coder *c);
int			coder_debug_refactor(t_coder *c);

/* monitor */
void		*monitor_routine(void *arg);

/* logging */
void		log_line(t_sim *sim, int coder_idx, const char *msg);
void		log_burnout(t_sim *sim, int coder_idx);
long long	log_compile_start(t_coder *c);

/* utils */
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

/* deadline (scheduler) */
long long	coder_deadline(t_sim *sim, t_coder *c);

#endif
