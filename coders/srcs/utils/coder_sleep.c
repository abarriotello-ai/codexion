/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   coder_sleep.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abarrio <abarrio@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/24 00:21:38 by abarrio           #+#    #+#             */
/*   Updated: 2026/09/24 00:21:38 by abarrio          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "coders.h"

// True once the gate is open, or the simulation was aborted before.
static int	gate_passed(t_sim *sim)
{
	int	passed;

	pthread_mutex_lock(&sim->stop_lock);
	passed = (sim->started || sim->stop_flag);
	pthread_mutex_unlock(&sim->stop_lock);
	return (passed);
}

// Every coder sleeps on its own condition variable, paired with its own
// state_lock. With one shared pair, all the coders whose sleep ends in the
// same millisecond would queue on one mutex, and the monitor behind them.
void	wake_sleepers(t_sim *sim)
{
	int	i;

	i = 0;
	while (i < sim->cfg.n_coders)
	{
		pthread_mutex_lock(&sim->coders[i].state_lock);
		pthread_cond_broadcast(&sim->coders[i].sleep_cond);
		pthread_mutex_unlock(&sim->coders[i].state_lock);
		i++;
	}
}

// Opened by the monitor once every thread exists: time 0 starts now for
// everyone at once, so slow thread creation never eats into a deadline.
void	open_start_gate(t_sim *sim)
{
	pthread_mutex_lock(&sim->stop_lock);
	sim->sim_start_ms = now_ms();
	sim->started = 1;
	pthread_mutex_unlock(&sim->stop_lock);
	wake_sleepers(sim);
}

// Returns 1 when the simulation starts, 0 if it was aborted before that.
int	wait_start_gate(t_coder *c)
{
	pthread_mutex_lock(&c->state_lock);
	while (!gate_passed(c->sim))
		pthread_cond_wait(&c->sleep_cond, &c->state_lock);
	pthread_mutex_unlock(&c->state_lock);
	return (!stop_requested(c->sim));
}

// Interruptible sleep until an absolute instant. Returns 0 if the full
// time elapsed, -1 if the simulation stopped first. The target is absolute,
// so wake-up latency never accumulates from one phase to the next.
int	coder_sleep_until(t_coder *c, long long target_ms)
{
	t_timespec	ts;
	int			stopped;

	ts = ms_to_timespec(target_ms);
	pthread_mutex_lock(&c->state_lock);
	stopped = stop_requested(c->sim);
	while (!stopped && now_ms() < target_ms)
	{
		pthread_cond_timedwait(&c->sleep_cond, &c->state_lock, &ts);
		stopped = stop_requested(c->sim);
	}
	pthread_mutex_unlock(&c->state_lock);
	if (stopped)
		return (-1);
	return (0);
}
