/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   dongle_release.c                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abarrio <abarrio@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/24 00:21:38 by abarrio           #+#    #+#             */
/*   Updated: 2026/09/24 00:21:38 by abarrio          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "coders.h"

static void	release_one(t_dongle *d, long long cooldown_ms)
{
	pthread_mutex_lock(&d->lock);
	d->state = DONGLE_COOLING_DOWN;
	d->cooldown_ready_at = now_ms() + cooldown_ms;
	pthread_mutex_unlock(&d->lock);
}

// After a compile: both dongles enter their cooldown, then the
// neighbours that may be waiting for them are woken up.
void	dongles_release(t_coder *c)
{
	pthread_mutex_lock(&c->sim->arb_lock);
	release_one(c->dongle_hi, c->sim->cfg.dongle_cooldown);
	release_one(c->dongle_lo, c->sim->cfg.dongle_cooldown);
	notify_neighbours(c);
	pthread_mutex_unlock(&c->sim->arb_lock);
}
