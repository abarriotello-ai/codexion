/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   sim_init.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abarrio <abarrio@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/24 00:21:38 by abarrio           #+#    #+#             */
/*   Updated: 2026/10/05 21:05:32 by abarrio          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "coders.h"

static int	init_sim_locks(t_sim *sim)
{
	if (pthread_mutex_init(&sim->stop_lock, NULL) != 0)
		return (-1);
	if (pthread_mutex_init(&sim->log_lock, NULL) != 0)
		return (pthread_mutex_destroy(&sim->stop_lock), -1);
	if (pthread_mutex_init(&sim->arb_lock, NULL) != 0)
	{
		pthread_mutex_destroy(&sim->log_lock);
		pthread_mutex_destroy(&sim->stop_lock);
		return (-1);
	}
	return (0);
}

static int	alloc_arrays(t_sim *sim)
{
	sim->dongles = malloc(sizeof(t_dongle) * sim->cfg.n_coders);
	if (!sim->dongles)
		return (-1);
	sim->coders = malloc(sizeof(t_coder) * sim->cfg.n_coders);
	if (!sim->coders)
	{
		free(sim->dongles);
		sim->dongles = NULL;
		return (-1);
	}
	memset(sim->dongles, 0, sizeof(t_dongle) * sim->cfg.n_coders);
	memset(sim->coders, 0, sizeof(t_coder) * sim->cfg.n_coders);
	return (0);
}

static int	init_one_dongle(t_dongle *d)
{
	d->state = DONGLE_AVAILABLE;
	d->cooldown_ready_at = 0;
	d->waitq.size = 0;
	d->waitq.capacity = WAITQ_CAPACITY;
	d->waitq.items = malloc(sizeof(t_heap_entry) * WAITQ_CAPACITY);
	if (!d->waitq.items)
		return (-1);
	if (pthread_mutex_init(&d->lock, NULL) != 0)
	{
		free(d->waitq.items);
		return (-1);
	}
	return (0);
}

static int	init_dongles(t_sim *sim)
{
	int	i;

	i = 0;
	while (i < sim->cfg.n_coders)
	{
		if (init_one_dongle(&sim->dongles[i]) != 0)
		{
			while (--i >= 0)
			{
				pthread_mutex_destroy(&sim->dongles[i].lock);
				free(sim->dongles[i].waitq.items);
			}
			return (-1);
		}
		i++;
	}
	return (0);
}

// Builds the shared part of the simulation: the global mutexes, the
// coders and dongles arrays, and every dongle (state, queue, lock).
// Each step undoes what was already created if it fails. Returns -1 then.
int	sim_init(t_sim *sim, t_config *cfg)
{
	memset(sim, 0, sizeof(t_sim));
	sim->cfg = *cfg;
	if (init_sim_locks(sim) != 0)
		return (-1);
	if (alloc_arrays(sim) != 0)
		return (sim_destroy_locks(sim), -1);
	if (init_dongles(sim) != 0)
	{
		free(sim->dongles);
		free(sim->coders);
		return (sim_destroy_locks(sim), -1);
	}
	return (0);
}
