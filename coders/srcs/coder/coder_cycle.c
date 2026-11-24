/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   coder_cycle.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abarrio <abarrio@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/24 00:21:38 by abarrio           #+#    #+#             */
/*   Updated: 2026/09/24 00:21:38 by abarrio          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "coders.h"

// Each phase ends at an absolute instant chained from the compile start,
// so wake-up latency is absorbed instead of piling up cycle after cycle.
// Returns -1 when the simulation stopped during the phase.
int	coder_do_compile(t_coder *c)
{
	long long	start;
	int			ret;

	if (dongles_acquire(c) != ACQUIRE_OK)
		return (-1);
	log_line(c->sim, c->idx, "has taken a dongle");
	log_line(c->sim, c->idx, "has taken a dongle");
	start = log_compile_start(c);
	ret = -1;
	if (start >= 0)
		ret = coder_sleep_until(c, start + c->sim->cfg.t_compile);
	dongles_release(c);
	return (ret);
}

// Counts a finished compile. Returns 1 once the coder reached
// compiles_required: it then stops and the monitor sees it as done.
int	coder_finish_compile(t_coder *c)
{
	int	done;

	pthread_mutex_lock(&c->state_lock);
	c->compiles_done++;
	done = (c->compiles_done >= c->sim->cfg.compiles_required);
	pthread_mutex_unlock(&c->state_lock);
	return (done);
}

// Debug then refactor. The end of each phase is computed from the
// compile start, like in coder_do_compile. -1 if stopped meanwhile.
int	coder_debug_refactor(t_coder *c)
{
	long long	phase_end;

	phase_end = get_last_compile_start(c) + c->sim->cfg.t_compile;
	log_line(c->sim, c->idx, "is debugging");
	phase_end += c->sim->cfg.t_debug;
	if (coder_sleep_until(c, phase_end) != 0)
		return (-1);
	log_line(c->sim, c->idx, "is refactoring");
	phase_end += c->sim->cfg.t_refactor;
	return (coder_sleep_until(c, phase_end));
}
