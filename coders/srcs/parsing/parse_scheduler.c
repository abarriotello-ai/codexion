/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parse_scheduler.c                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abarrio <abarrio@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/24 00:21:38 by abarrio           #+#    #+#             */
/*   Updated: 2026/09/28 18:43:30 by abarrio          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "coders.h"

int	parse_scheduler(const char *s, t_scheduler *out)
{
	if (!s)
		return (0);
	if (strcmp(s, "fifo") == 0)
	{
		*out = SCHED_MODE_FIFO;
		return (1);
	}
	if (strcmp(s, "edf") == 0)
	{
		*out = SCHED_MODE_EDF;
		return (1);
	}
	return (0);
}
