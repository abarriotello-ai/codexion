/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: user <user@student.42.fr>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/23 20:53:59 by abarrio           #+#    #+#             */
/*   Updated: 2026/11/23 20:13:31 by user             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "coders.h"

// Creates one thread per coder. `started` tells how many really exist,
// so that only those are joined if a creation fails halfway.
static int	start_coders(t_sim *sim, int *started)
{
	*started = 0;
	while (*started < sim->cfg.n_coders)
	{
		if (pthread_create(&sim->coders[*started].thread, NULL,
				coder_routine, &sim->coders[*started]) != 0)
			return (-1);
		(*started)++;
	}
	return (0);
}

static void	join_coders(t_sim *sim, int count)
{
	int	i;

	i = 0;
	while (i < count)
	{
		pthread_join(sim->coders[i].thread, NULL);
		i++;
	}
}

// Coders wait at the start gate, which the monitor opens once it runs. If
// any pthread_create fails the gate never opens: the coders already created
// see the stop and exit without printing a single line.
static int	run_simulation(t_sim *sim)
{
	int	started;

	if (start_coders(sim, &started) != 0
		|| pthread_create(&sim->monitor_thread, NULL,
			monitor_routine, sim) != 0)
	{
		stop_simulation(sim);
		join_coders(sim, started);
		fprintf(stderr, "codexion: failed to create threads\n");
		return (1);
	}
	join_coders(sim, started);
	pthread_join(sim->monitor_thread, NULL);
	return (0);
}

// Program workflow:
//   1. parse and validate the 8 arguments into a t_config;
//   2. build the simulation (locks, dongles, coders);
//   3. run it: coder threads + monitor thread, then join them all;
//   4. free everything. Returns 1 on any error, 0 otherwise.
int	main(int argc, char **argv)
{
	t_config	cfg;
	t_sim		sim;
	int			ret;

	if (parse_args(argc, argv, &cfg) != 0)
		return (1);
	if (sim_init(&sim, &cfg) != 0)
		return (fprintf(stderr, "codexion: initialization failed\n"), 1);
	if (coder_init(&sim) != 0)
	{
		sim_destroy(&sim);
		return (fprintf(stderr, "codexion: initialization failed\n"), 1);
	}
	ret = run_simulation(&sim);
	sim_destroy(&sim);
	return (ret);
}
