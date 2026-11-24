/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   coder_state.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abarrio <abarrio@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/24 00:21:38 by abarrio           #+#    #+#             */
/*   Updated: 2026/09/24 00:21:38 by abarrio          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "coders.h"

// Thread-safe read of last_compile_start (-1 before the first compile).
long long	get_last_compile_start(t_coder *c)
{
	long long	ts;

	pthread_mutex_lock(&c->state_lock);
	ts = c->last_compile_start;
	pthread_mutex_unlock(&c->state_lock);
	return (ts);
}
