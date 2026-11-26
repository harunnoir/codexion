/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   init_threads.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abait-el <abait-el@student.1337.ma>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/10 02:28:12 by abait-el          #+#    #+#             */
/*   Updated: 2026/09/10 02:28:13 by abait-el         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <string.h>
#include "codexion.h"

static void	release_start(t_simulation *simulation)
{
	pthread_mutex_lock(&simulation->state_mutex);
	simulation->started_at_ms = clock_now_ms();
	simulation->start_released = true;
	pthread_cond_broadcast(&simulation->state_changed);
	pthread_mutex_unlock(&simulation->state_mutex);
}

static void	stop_created_threads(t_simulation *simulation, int created)
{
	int	index;

	pthread_mutex_lock(&simulation->state_mutex);
	simulation->is_stopped = true;
	simulation->start_released = true;
	pthread_cond_broadcast(&simulation->state_changed);
	pthread_cond_broadcast(&simulation->sleep_condition);
	pthread_mutex_unlock(&simulation->state_mutex);
	scheduler_wake_all(simulation);
	index = 0;
	while (index < created)
		pthread_join(simulation->coders[index++].thread, NULL);
}

static int	start_coder_threads(t_simulation *simulation)
{
	int	created;

	created = 0;
	while (created < simulation->config.coder_count)
	{
		if (pthread_create(&simulation->coders[created].thread, NULL,
				coder_thread, &simulation->coders[created]) != 0)
		{
			stop_created_threads(simulation, created);
			return (-1);
		}
		created++;
	}
	return (0);
}

static int	start_threads(t_simulation *simulation)
{
	if (start_coder_threads(simulation) != 0)
		return (-1);
	if (pthread_create(&simulation->monitor_thread, NULL,
			monitor_thread, simulation) != 0)
	{
		stop_created_threads(simulation, simulation->config.coder_count);
		return (-1);
	}
	release_start(simulation);
	return (0);
}

int	simulation_init(t_simulation *simulation, const t_config *config)
{
	memset(simulation, 0, sizeof(*simulation));
	simulation->config = *config;
	if (simulation_init_sync(simulation) != 0)
		return (-1);
	if (simulation_init_data(simulation) != 0)
	{
		simulation_destroy_sync(simulation);
		return (-1);
	}
	if (start_threads(simulation) != 0)
	{
		simulation_destroy(simulation);
		return (-1);
	}
	return (0);
}
