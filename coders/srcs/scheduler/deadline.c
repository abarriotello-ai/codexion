/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   deadline.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abarrio <abarrio@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/24 00:21:38 by abarrio           #+#    #+#             */
/*   Updated: 2026/09/28 11:57:03 by abarrio          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "coders.h"

// Instant at which the coder burns out unless it starts compiling:
// last compile start (or simulation start) + t_burnout.
long long	coder_deadline(t_sim *sim, t_coder *c)
{
	long long	baseline;

	baseline = get_last_compile_start(c);
	if (baseline < 0)
		baseline = sim->sim_start_ms;
	return (baseline + sim->cfg.t_burnout);
}

// Priority of a coder in a dongle queue, the lowest goes first:
// its burnout deadline in EDF, its arrival ticket in FIFO.
long long	compute_key(t_coder *c, long long ticket)
{
	if (c->sim->cfg.scheduler == SCHED_MODE_EDF)
		return (coder_deadline(c->sim, c));
	return (ticket);
}
