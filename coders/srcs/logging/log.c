/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   log.c                                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abarrio <abarrio@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/24 00:21:38 by abarrio           #+#    #+#             */
/*   Updated: 2026/09/24 00:21:38 by abarrio          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "coders.h"

// The timestamp is taken before the stop check, both under log_lock. A
// printed line therefore always predates the stop, hence the burnout line:
// timestamps come out sorted and nothing ever follows "burned out".
void	log_line(t_sim *sim, int coder_idx, const char *msg)
{
	long long	now;

	pthread_mutex_lock(&sim->log_lock);
	now = now_ms();
	if (!stop_requested(sim))
		printf("%lld %d %s\n", now - sim->sim_start_ms, coder_idx + 1, msg);
	pthread_mutex_unlock(&sim->log_lock);
}

// Only the monitor calls this, once. The stop is raised before queueing for
// log_lock, so coders waiting on that lock give up without printing and the
// monitor is not stuck behind them; the line carries the detection time.
// The caller then wakes everyone with stop_simulation.
void	log_burnout(t_sim *sim, int coder_idx)
{
	long long	now;

	set_stop(sim);
	now = now_ms();
	pthread_mutex_lock(&sim->log_lock);
	printf("%lld %d burned out\n", now - sim->sim_start_ms, coder_idx + 1);
	pthread_mutex_unlock(&sim->log_lock);
}

// "is compiling" is printed and becomes the new last_compile_start in one
// step, with one timestamp: the deadline the monitor checks, the length of
// the compile and the log line then agree by construction. The store and
// the stop check share state_lock with the monitor's deadline check, so
// either the coder starts first (and the monitor sees the new deadline) or
// the monitor stops the simulation first (and nothing is printed). Returns
// the timestamp, or -1 if the simulation had stopped.
long long	log_compile_start(t_coder *c)
{
	long long	now;
	int			running;

	pthread_mutex_lock(&c->sim->log_lock);
	now = now_ms();
	pthread_mutex_lock(&c->state_lock);
	running = !stop_requested(c->sim);
	if (running)
		c->last_compile_start = now;
	pthread_mutex_unlock(&c->state_lock);
	if (running)
		printf("%lld %d is compiling\n", now - c->sim->sim_start_ms,
			c->idx + 1);
	pthread_mutex_unlock(&c->sim->log_lock);
	if (!running)
		return (-1);
	return (now);
}
