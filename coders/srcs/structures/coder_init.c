/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   coder_init.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abarrio <abarrio@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/24 00:21:38 by abarrio           #+#    #+#             */
/*   Updated: 2026/09/24 00:21:38 by abarrio          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "coders.h"

// Round table: coder i sits between dongle i-1 (left) and dongle i
// (right). They are stored lowest index first. With a single coder both
// pointers are the same dongle, which coder_routine detects.
static void	resolve_dongles(t_sim *sim, int i)
{
	int		left;
	int		right;
	t_coder	*c;

	left = (i - 1 + sim->cfg.n_coders) % sim->cfg.n_coders;
	right = i;
	c = &sim->coders[i];
	if (left < right)
	{
		c->dongle_lo = &sim->dongles[left];
		c->dongle_hi = &sim->dongles[right];
	}
	else
	{
		c->dongle_lo = &sim->dongles[right];
		c->dongle_hi = &sim->dongles[left];
	}
}

static int	init_coder_sync(t_coder *c)
{
	if (pthread_mutex_init(&c->state_lock, NULL) != 0)
		return (-1);
	if (pthread_cond_init(&c->wake, NULL) != 0)
		return (pthread_mutex_destroy(&c->state_lock), -1);
	if (pthread_cond_init(&c->sleep_cond, NULL) != 0)
	{
		pthread_cond_destroy(&c->wake);
		pthread_mutex_destroy(&c->state_lock);
		return (-1);
	}
	return (0);
}

void	destroy_coder_sync(t_coder *c)
{
	pthread_cond_destroy(&c->sleep_cond);
	pthread_cond_destroy(&c->wake);
	pthread_mutex_destroy(&c->state_lock);
}

// Fills every coder (index, no compile yet, its dongles) and creates
// its mutex and condition variables. Rolls back on failure.
int	coder_init(t_sim *sim)
{
	int	i;

	i = 0;
	while (i < sim->cfg.n_coders)
	{
		sim->coders[i].idx = i;
		sim->coders[i].last_compile_start = -1;
		sim->coders[i].compiles_done = 0;
		sim->coders[i].sim = sim;
		resolve_dongles(sim, i);
		if (init_coder_sync(&sim->coders[i]) != 0)
		{
			while (--i >= 0)
				destroy_coder_sync(&sim->coders[i]);
			return (-1);
		}
		i++;
	}
	return (0);
}
