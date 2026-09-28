/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parse_number.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abarrio <abarrio@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/24 00:21:38 by abarrio           #+#    #+#             */
/*   Updated: 2026/09/28 18:43:24 by abarrio          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "coders.h"

int	parse_number(const char *s, long long *out, long long max)
{
	long long	val;
	int			i;

	if (!s || s[0] == '\0')
		return (0);
	i = 0;
	val = 0;
	while (s[i])
	{
		if (s[i] < '0' || s[i] > '9')
			return (0);
		val = val * 10 + (s[i] - '0');
		if (val > max)
			return (0);
		i++;
	}
	*out = val;
	return (1);
}
