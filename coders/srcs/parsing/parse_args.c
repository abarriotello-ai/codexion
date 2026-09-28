/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parse_args.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abarrio <abarrio@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/28 18:43:03 by abarrio           #+#    #+#             */
/*   Updated: 2026/09/28 18:43:11 by abarrio          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "coders.h"

static void	print_usage(void)
{
	fprintf(stderr, "Usage: ./codexion number_of_coders time_to_burnout "
		"time_to_compile time_to_debug time_to_refactor "
		"number_of_compiles_required dongle_cooldown scheduler(fifo|edf)\n");
}

static int	parse_int_field(const char *s, int *out, long long max, int zero)
{
	long long	val;

	if (!parse_number(s, &val, max))
		return (0);
	if (val == 0 && !zero)
		return (0);
	*out = (int)val;
	return (1);
}

static int	parse_ms_field(const char *s, long long *out, int zero)
{
	if (!parse_number(s, out, MAX_MS))
		return (0);
	if (*out == 0 && !zero)
		return (0);
	return (1);
}

int	parse_args(int argc, char **argv, t_config *cfg)
{
	if (argc != 9)
		return (print_usage(), -1);
	if (!parse_int_field(argv[1], &cfg->n_coders, MAX_CODERS, 0))
		return (print_usage(), -1);
	if (!parse_ms_field(argv[2], &cfg->t_burnout, 0))
		return (print_usage(), -1);
	if (!parse_ms_field(argv[3], &cfg->t_compile, 0))
		return (print_usage(), -1);
	if (!parse_ms_field(argv[4], &cfg->t_debug, 1))
		return (print_usage(), -1);
	if (!parse_ms_field(argv[5], &cfg->t_refactor, 1))
		return (print_usage(), -1);
	if (!parse_int_field(argv[6], &cfg->compiles_required, INT_MAX, 1))
		return (print_usage(), -1);
	if (!parse_ms_field(argv[7], &cfg->dongle_cooldown, 1))
		return (print_usage(), -1);
	if (!parse_scheduler(argv[8], &cfg->scheduler))
		return (print_usage(), -1);
	return (0);
}
