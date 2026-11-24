/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   sim_stop.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abarrio <abarrio@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/24 00:21:38 by abarrio           #+#    #+#             */
/*   Updated: 2026/09/24 00:21:38 by abarrio          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "coders.h"

// Thread-safe read and write of the stop flag.
int	stop_requested(t_sim *sim)
{
	int	flag;

	pthread_mutex_lock(&sim->stop_lock);
	flag = sim->stop_flag;
	pthread_mutex_unlock(&sim->stop_lock);
	return (flag);
}

void	set_stop(t_sim *sim)
{
	pthread_mutex_lock(&sim->stop_lock);
	sim->stop_flag = 1;
	pthread_mutex_unlock(&sim->stop_lock);
}

// Wakes the coders parked in dongles_acquire (each on its `wake`
// condition, paired with arb_lock).
void	wake_dongle_waiters(t_sim *sim)
{
	int	i;

	pthread_mutex_lock(&sim->arb_lock);
	i = 0;
	while (i < sim->cfg.n_coders)
	{
		pthread_cond_broadcast(&sim->coders[i].wake);
		i++;
	}
	pthread_mutex_unlock(&sim->arb_lock);
}

// Raises the flag, then wakes every thread that could be waiting: sleeping
// in a phase, parked at the start gate or waiting for dongles. Each one
// re-checks the flag after waking, so nobody oversleeps the stop.
void	stop_simulation(t_sim *sim)
{
	set_stop(sim);
	wake_sleepers(sim);
	wake_dongle_waiters(sim);
}

// Used by the lone coder: blocks until the simulation stops.
void	wait_until_stop(t_coder *c)
{
	pthread_mutex_lock(&c->state_lock);
	while (!stop_requested(c->sim))
		pthread_cond_wait(&c->sleep_cond, &c->state_lock);
	pthread_mutex_unlock(&c->state_lock);
}
