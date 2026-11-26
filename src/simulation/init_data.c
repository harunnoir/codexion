/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   init_data.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abait-el <abait-el@student.1337.ma>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/10 02:28:08 by abait-el          #+#    #+#             */
/*   Updated: 2026/09/10 02:28:09 by abait-el         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <stdlib.h>
#include "codexion.h"

static void	destroy_dongle_mutexes(t_simulation *simulation, int count)
{
	int	index;

	index = 0;
	while (index < count)
		pthread_mutex_destroy(&simulation->dongles[index++].mutex);
}

static int	init_dongles(t_simulation *simulation)
{
	t_request_compare	compare;
	int					index;

	compare = scheduler_comparator(simulation->config.scheduler);
	index = 0;
	while (index < simulation->config.coder_count)
	{
		simulation->dongles[index].id = index;
		simulation->dongles[index].is_held = false;
		simulation->dongles[index].available_at_ms = 0;
		request_heap_init(&simulation->dongles[index].waiters, compare);
		if (pthread_mutex_init(&simulation->dongles[index].mutex, NULL) != 0)
		{
			destroy_dongle_mutexes(simulation, index);
			return (-1);
		}
		index++;
	}
	return (0);
}

static void	init_coders(t_simulation *simulation)
{
	t_coder	*coder;
	int		index;

	index = 0;
	while (index < simulation->config.coder_count)
	{
		coder = &simulation->coders[index];
		coder->id = index + 1;
		coder->simulation = simulation;
		coder->has_dongles = false;
		coder->has_pending_request = false;
		coder->compile_count = 0;
		coder->burnout_deadline_ms = simulation->config.burnout_ms;
		coder->is_finished = false;
		coder->has_burned_out = false;
		index++;
	}
}

static void	link_dongles(t_simulation *simulation)
{
	int	index;
	int	left;

	index = 0;
	while (index < simulation->config.coder_count)
	{
		left = (index + simulation->config.coder_count - 1)
			% simulation->config.coder_count;
		simulation->coders[index].left_dongle
			= &simulation->dongles[left];
		simulation->coders[index].right_dongle
			= &simulation->dongles[index];
		index++;
	}
}

int	simulation_init_data(t_simulation *simulation)
{
	simulation->dongles = malloc(sizeof(t_dongle)
			* simulation->config.coder_count);
	simulation->coders = malloc(sizeof(t_coder)
			* simulation->config.coder_count);
	if (!simulation->dongles || !simulation->coders)
	{
		free(simulation->dongles);
		free(simulation->coders);
		return (-1);
	}
	if (init_dongles(simulation) != 0)
	{
		free(simulation->dongles);
		free(simulation->coders);
		return (-1);
	}
	init_coders(simulation);
	link_dongles(simulation);
	return (0);
}
