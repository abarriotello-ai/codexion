/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   dongle_cooldown.c                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abarrio <abarrio@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/24 00:21:38 by abarrio           #+#    #+#             */
/*   Updated: 2026/09/24 00:21:38 by abarrio          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "coders.h"

// 1 if the dongle can be taken. Also ends an expired cooldown, turning
// COOLING_DOWN back into AVAILABLE.
int	dongle_available(t_dongle *d)
{
	int	available;

	pthread_mutex_lock(&d->lock);
	if (d->state == DONGLE_COOLING_DOWN && now_ms() >= d->cooldown_ready_at)
		d->state = DONGLE_AVAILABLE;
	available = (d->state == DONGLE_AVAILABLE);
	pthread_mutex_unlock(&d->lock);
	return (available);
}

// The instant this dongle's cooldown ends, or 0 if it is not cooling down.
long long	dongle_cooling_until(t_dongle *d)
{
	long long	ready_at;

	ready_at = 0;
	pthread_mutex_lock(&d->lock);
	if (d->state == DONGLE_COOLING_DOWN && now_ms() < d->cooldown_ready_at)
		ready_at = d->cooldown_ready_at;
	pthread_mutex_unlock(&d->lock);
	return (ready_at);
}
