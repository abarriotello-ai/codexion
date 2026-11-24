/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   monitor.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abarrio <abarrio@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/24 00:21:38 by abarrio           #+#    #+#             */
/*   Updated: 2026/09/24 00:21:38 by abarrio          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "coders.h"

// Same as coder_deadline, for a caller already holding state_lock.
static long long	coder_deadline_locked(t_sim *sim, t_coder *c)
{
	long long	baseline;

	if (c->last_compile_start >= 0)
		baseline = c->last_compile_start;
	else
		baseline = sim->sim_start_ms;
	return (baseline + sim->cfg.t_burnout);
}

// A burnout raises the stop while state_lock is still held: the same lock
// under which log_compile_start stores a new compile start and checks the
// stop. So a coder can never start compiling after being declared burned
// out, nor be declared burned out after it started compiling.
static int	coder_check(t_sim *sim, t_coder *c, long long now,
				long long *earliest)
{
	long long	deadline;
	int			status;

	pthread_mutex_lock(&c->state_lock);
	deadline = coder_deadline_locked(sim, c);
	status = 0;
	if (c->compiles_done >= sim->cfg.compiles_required)
		status = 1;
	else if (now >= deadline)
	{
		status = 2;
		set_stop(sim);
	}
	pthread_mutex_unlock(&c->state_lock);
	if (status == 0 && deadline < *earliest)
		*earliest = deadline;
	return (status);
}

// One pass over every coder. Returns 1 if all of them are done.
// On a burnout it stops at once and sets burned_idx. earliest receives
// the closest deadline among the coders still running.
static int	scan_once(t_sim *sim, long long *earliest, int *burned_idx)
{
	int	i;
	int	all_done;
	int	status;

	i = 0;
	all_done = 1;
	*earliest = LLONG_MAX;
	*burned_idx = -1;
	while (i < sim->cfg.n_coders)
	{
		status = coder_check(sim, &sim->coders[i], now_ms(), earliest);
		if (status == 2)
		{
			*burned_idx = i;
			return (0);
		}
		if (status == 0)
			all_done = 0;
		i++;
	}
	return (all_done);
}

// Sleeps until target_ms, but never longer than POLL_CAP_MS.
static void	sleep_until(long long target_ms)
{
	long long	now;
	long long	cap;
	long long	wait_ms;

	now = now_ms();
	cap = now + POLL_CAP_MS;
	if (target_ms > cap)
		target_ms = cap;
	wait_ms = target_ms - now;
	if (wait_ms > 0)
		usleep(wait_ms * 1000);
}

// Thread of the monitor. Opens the start gate, then loops:
//   - scan every coder;
//   - a coder burned out: print it and stop the simulation;
//   - every coder reached compiles_required: stop the simulation;
//   - otherwise sleep until the closest deadline (capped) and rescan.
void	*monitor_routine(void *arg)
{
	t_sim		*sim;
	long long	earliest;
	int			burned_idx;
	int			all_done;

	sim = (t_sim *)arg;
	open_start_gate(sim);
	while (!stop_requested(sim))
	{
		all_done = scan_once(sim, &earliest, &burned_idx);
		if (burned_idx != -1)
		{
			log_burnout(sim, burned_idx);
			stop_simulation(sim);
			break ;
		}
		if (all_done)
		{
			stop_simulation(sim);
			break ;
		}
		sleep_until(earliest);
	}
	return (NULL);
}
