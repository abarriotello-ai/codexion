/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   dongle_acquire.c                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abarrio <abarrio@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/24 00:21:38 by abarrio           #+#    #+#             */
/*   Updated: 2026/09/24 00:21:38 by abarrio          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "coders.h"

// Both dongles of the coder are free (not held, cooldown over).
static int	coder_eligible(t_coder *c)
{
	return (dongle_available(c->dongle_lo)
		&& dongle_available(c->dongle_hi));
}

// Must coder c leave dongle d to the coder at the head of d's queue?
//   - no if the queue is empty or c itself is the head;
//   - yes if the head could take its own pair right now;
//   - EDF only: yes if the head is about to burn out. Taking d now would
//     make it wait one compile + one cooldown more, which it may not
//     survive, so d is left free for it even if it cannot take it yet.
static int	must_yield(t_coder *c, t_dongle *d)
{
	t_heap_entry	head;
	t_config		*cfg;
	long long		slack;

	if (!dongle_head(d, &head) || head.coder_idx == c->idx)
		return (0);
	if (coder_eligible(&c->sim->coders[head.coder_idx]))
		return (1);
	cfg = &c->sim->cfg;
	if (cfg->scheduler != SCHED_MODE_EDF)
		return (0);
	slack = head.key - now_ms();
	return (slack < cfg->t_compile + cfg->dongle_cooldown + URGENCY_MARGIN_MS);
}

// Takes both dongles at once or none: a coder never holds a single
// dongle while waiting for the other, so no deadlock is possible.
// Neighbours are notified because the queue heads changed.
static int	try_take_pair(t_coder *c)
{
	if (!coder_eligible(c) || must_yield(c, c->dongle_lo)
		|| must_yield(c, c->dongle_hi))
		return (0);
	dongle_take(c->dongle_lo, c->idx);
	dongle_take(c->dongle_hi, c->idx);
	notify_neighbours(c);
	return (1);
}

// Draws an arrival ticket and joins the queues of both dongles with the
// same key. Neighbours are notified as the queue heads may have changed.
static void	enqueue_pair(t_coder *c)
{
	long long	ticket;
	long long	key;

	ticket = c->sim->next_ticket++;
	key = compute_key(c, ticket);
	dongle_enqueue(c->dongle_lo, c->idx, key, ticket);
	dongle_enqueue(c->dongle_hi, c->idx, key, ticket);
	notify_neighbours(c);
}

// Gets both dongles of the coder, or gives up when the simulation stops.
// Workflow, entirely under arb_lock:
//   1. join the wait queue of both dongles;
//   2. try to take the pair; while impossible, sleep on `wake` (until a
//      neighbour changes something or a cooldown ends) and retry;
//   3. on stop, leave both queues so that nobody yields to a ghost.
// Returns ACQUIRE_OK when both are held, ACQUIRE_ABORTED on stop.
int	dongles_acquire(t_coder *c)
{
	int	got;

	pthread_mutex_lock(&c->sim->arb_lock);
	enqueue_pair(c);
	got = try_take_pair(c);
	while (!got && !stop_requested(c->sim))
	{
		wait_for_dongles(c);
		got = try_take_pair(c);
	}
	if (!got)
	{
		dongle_dequeue(c->dongle_lo, c->idx);
		dongle_dequeue(c->dongle_hi, c->idx);
	}
	pthread_mutex_unlock(&c->sim->arb_lock);
	if (got)
		return (ACQUIRE_OK);
	return (ACQUIRE_ABORTED);
}
