/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   coder_routine.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abarrio <abarrio@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/24 00:21:38 by abarrio           #+#    #+#             */
/*   Updated: 2026/09/24 00:21:38 by abarrio          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "coders.h"

// With one coder there is one dongle: it can be taken but a second one
// never exists, so the coder holds it until the monitor declares the
// burnout. dongles_acquire must not be used here: it would wait forever.
static void	lone_coder(t_coder *c)
{
	log_line(c->sim, c->idx, "has taken a dongle");
	wait_until_stop(c);
}

// Thread of a coder. Workflow:
//   wait at the start gate (or leave if the start was aborted);
//   nothing to do if 0 compiles are required; lone coder special case;
//   then loop: compile (takes and releases the dongles) -> count the
//   compile, leave once compiles_required is reached -> debug -> refactor.
//   Any phase interrupted by the stop ends the thread.
void	*coder_routine(void *arg)
{
	t_coder	*c;

	c = (t_coder *)arg;
	if (!wait_start_gate(c))
		return (NULL);
	if (c->sim->cfg.compiles_required == 0)
		return (NULL);
	if (c->dongle_lo == c->dongle_hi)
		return (lone_coder(c), NULL);
	while (!stop_requested(c->sim))
	{
		if (coder_do_compile(c) != 0)
			break ;
		if (coder_finish_compile(c))
			break ;
		if (coder_debug_refactor(c) != 0)
			break ;
	}
	return (NULL);
}
