/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parse_args.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abarrio <abarrio@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/28 18:43:03 by abarrio           #+#    #+#             */
/*   Updated: 2026/09/28 19:42:21 by abarrio          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "coders.h"

static void	print_usage(void)
{
	fprintf(stderr, "Usage: ./codexion number_of_coders time_to_burnout "
		"time_to_compile time_to_debug time_to_refactor "
		"number_of_compiles_required dongle_cooldown scheduler(fifo|edf)\n");
}

static int	parse_int_field(const char *s, int *out, long long min,
		long long max)
{
	long long	val;

	if (!parse_number(s, &val, max))
		return (0);
	if (val < min)
		return (0);
	*out = (int)val;
	return (1);
}

static int	parse_ms_field(const char *s, long long *out, long long min)
{
	if (!parse_number(s, out, MAX_MS))
		return (0);
	if (*out < min)
		return (0);
	return (1);
}

// Validates the 8 arguments one by one and fills cfg. On the first
// invalid value it prints the usage on stderr and returns -1.
// Ranges: 1..MAX_CODERS coders, every other number >= 0 (0 coders is no
// simulation, but a 0 ms time is a valid, if extreme, value).
int	parse_args(int argc, char **argv, t_config *cfg)
{
	if (argc != 9)
		return (print_usage(), -1);
	if (!parse_int_field(argv[1], &cfg->n_coders, 1, MAX_CODERS))
		return (print_usage(), -1);
	if (!parse_ms_field(argv[2], &cfg->t_burnout, 0))
		return (print_usage(), -1);
	if (!parse_ms_field(argv[3], &cfg->t_compile, 0))
		return (print_usage(), -1);
	if (!parse_ms_field(argv[4], &cfg->t_debug, 0))
		return (print_usage(), -1);
	if (!parse_ms_field(argv[5], &cfg->t_refactor, 0))
		return (print_usage(), -1);
	if (!parse_int_field(argv[6], &cfg->compiles_required, 0,
			INT_MAX))
		return (print_usage(), -1);
	if (!parse_ms_field(argv[7], &cfg->dongle_cooldown, 0))
		return (print_usage(), -1);
	if (!parse_scheduler(argv[8], &cfg->scheduler))
		return (print_usage(), -1);
	return (0);
}
