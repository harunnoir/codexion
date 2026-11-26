/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   config_parse.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abait-el <abait-el@student.1337.ma>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/10 02:27:30 by abait-el          #+#    #+#             */
/*   Updated: 2026/09/10 03:58:07 by abait-el         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"
#include <limits.h>
#include <stdio.h>
#include <string.h>

static long	parse_number(const char *text)
{
	long	value;

	if (!text || !*text || *text == '-')
		return (-1);
	if (*text == '+')
		text++;
	if (!*text)
		return (-1);
	value = 0;
	while (*text)
	{
		if (*text < '0' || *text > '9')
			return (-1);
		if (value > (LONG_MAX - (*text - '0')) / 10)
			return (-1);
		value = value * 10 + (*text++ - '0');
	}
	return (value);
}

static t_scheduler_policy	parse_scheduler(const char *text)
{
	if (strcmp(text, "fifo") == 0)
		return (POLICY_FIFO);
	if (strcmp(text, "edf") == 0)
		return (POLICY_EDF);
	return ((t_scheduler_policy)-1);
}

static int	print_usage(void)
{
	fprintf(stderr, "usage: ./codexion <number_of_coders> <time_to_burnout> "
		"<time_to_compile> <time_to_debug> <time_to_refactor> "
		"<number_of_compiles_required> <dongle_cooldown> <scheduler>\n"
		"\n"
		"  <number_of_coders>              coders in the ring\n"
		"  <time_to_burnout>               ms without starting a compile\n"
		"  <time_to_compile>               ms a compile takes\n"
		"  <time_to_debug>                 ms a debugging phase takes\n"
		"  <time_to_refactor>              ms a refactoring phase takes\n"
		"  <number_of_compiles_required>   compiles each coder needs to finish\n"
		"  <dongle_cooldown>               ms a dongle stays unavailable\n"
		"  <scheduler>                     \"fifo\" or \"edf\"\n");
	return (-1);
}

static void	store_config(t_config *config, long *values)
{
	config->coder_count = (int)values[0];
	config->burnout_ms = values[1];
	config->compile_ms = values[2];
	config->debug_ms = values[3];
	config->refactor_ms = values[4];
	config->required_compiles = values[5];
	config->cooldown_ms = values[6];
}

int	config_parse(t_config *config, int argc, char **argv)
{
	long	values[7];
	int		index;

	if (argc != 9)
		return (print_usage());
	index = 0;
	while (index < 7)
	{
		values[index] = parse_number(argv[index + 1]);
		if (values[index++] < 0)
			return (print_usage());
	}
	if (values[0] < 1 || values[0] > INT_MAX || values[1] < 1 || values[5] < 1)
		return (print_usage());
	store_config(config, values);
	config->scheduler = parse_scheduler(argv[8]);
	if ((int)config->scheduler < 0)
		return (print_usage());
	return (0);
}
