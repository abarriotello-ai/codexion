/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   dongle_wait.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abarrio <abarrio@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/24 00:21:38 by abarrio           #+#    #+#             */
/*   Updated: 2026/09/24 00:21:38 by abarrio          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "coders.h"

// A waiting coder's decision reads its two dongles, the heads of their
// queues (its neighbours) and each head's other dongle. So a change made by
// coder i can only matter to coders at distance 1 or 2: wake exactly those,
// instead of every waiter at the table. Called with arb_lock held.
void	notify_neighbours(t_coder *c)
{
	t_coder	*coders;
	int		n;
	int		dist;

	coders = c->sim->coders;
	n = c->sim->cfg.n_coders;
	dist = 1;
	while (dist <= 2)
	{
		pthread_cond_broadcast(&coders[(c->idx + dist) % n].wake);
		pthread_cond_broadcast(&coders[(c->idx - dist + 2 * n) % n].wake);
		dist++;
	}
}

// Parks the coder (arb_lock held) until a neighbour notifies it. The end
// of a cooldown is the one event nobody signals, so while one of its
// dongles is cooling down the wait is timed to end exactly then.
void	wait_for_dongles(t_coder *c)
{
	long long	lo;
	long long	hi;
	long long	wake_at;
	t_timespec	ts;

	lo = dongle_cooling_until(c->dongle_lo);
	hi = dongle_cooling_until(c->dongle_hi);
	wake_at = lo;
	if (hi > 0 && (wake_at == 0 || hi < wake_at))
		wake_at = hi;
	if (wake_at == 0)
	{
		pthread_cond_wait(&c->wake, &c->sim->arb_lock);
		return ;
	}
	ts = ms_to_timespec(wake_at);
	pthread_cond_timedwait(&c->wake, &c->sim->arb_lock, &ts);
}
