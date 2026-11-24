/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   dongle_queue.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abarrio <abarrio@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/24 00:21:38 by abarrio           #+#    #+#             */
/*   Updated: 2026/09/24 00:21:38 by abarrio          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "coders.h"

// Thread-safe wrappers around a dongle's wait queue (heap), each one
// taking the dongle's own lock.
void	dongle_enqueue(t_dongle *d, int coder_idx, long long key,
			long long ticket)
{
	pthread_mutex_lock(&d->lock);
	heap_push(&d->waitq, coder_idx, key, ticket);
	pthread_mutex_unlock(&d->lock);
}

void	dongle_dequeue(t_dongle *d, int coder_idx)
{
	pthread_mutex_lock(&d->lock);
	heap_remove(&d->waitq, coder_idx);
	pthread_mutex_unlock(&d->lock);
}

// Copies the first waiting coder into out. Returns 0 if nobody waits.
int	dongle_head(t_dongle *d, t_heap_entry *out)
{
	int	found;

	pthread_mutex_lock(&d->lock);
	found = (d->waitq.size > 0);
	if (found)
		*out = d->waitq.items[0];
	pthread_mutex_unlock(&d->lock);
	return (found);
}

// The coder leaves the queue and becomes the holder of the dongle.
void	dongle_take(t_dongle *d, int coder_idx)
{
	pthread_mutex_lock(&d->lock);
	heap_remove(&d->waitq, coder_idx);
	d->state = DONGLE_HELD;
	pthread_mutex_unlock(&d->lock);
}
