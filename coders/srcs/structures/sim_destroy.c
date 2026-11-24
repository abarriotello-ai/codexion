/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   sim_destroy.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abarrio <abarrio@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/24 00:21:38 by abarrio           #+#    #+#             */
/*   Updated: 2026/10/05 21:05:44 by abarrio          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "coders.h"

static void	destroy_dongles(t_sim *sim)
{
	int	i;

	i = 0;
	while (i < sim->cfg.n_coders)
	{
		pthread_mutex_destroy(&sim->dongles[i].lock);
		free(sim->dongles[i].waitq.items);
		i++;
	}
	free(sim->dongles);
}

static void	destroy_coders(t_sim *sim)
{
	int	i;

	i = 0;
	while (i < sim->cfg.n_coders)
	{
		destroy_coder_sync(&sim->coders[i]);
		i++;
	}
	free(sim->coders);
}

void	sim_destroy_locks(t_sim *sim)
{
	pthread_mutex_destroy(&sim->arb_lock);
	pthread_mutex_destroy(&sim->log_lock);
	pthread_mutex_destroy(&sim->stop_lock);
}

// Frees every resource. Only called once every thread has been joined.
void	sim_destroy(t_sim *sim)
{
	destroy_dongles(sim);
	destroy_coders(sim);
	sim_destroy_locks(sim);
}
