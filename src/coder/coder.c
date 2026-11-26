/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   coder.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abait-el <abait-el@student.1337.ma>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/10 02:27:26 by abait-el          #+#    #+#             */
/*   Updated: 2026/09/10 02:27:27 by abait-el         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"
#include <stdio.h>

static bool	coder_should_exit(t_coder *coder)
{
	t_simulation	*simulation;
	bool			should_exit;

	simulation = coder->simulation;
	pthread_mutex_lock(&simulation->state_mutex);
	if (coder->compile_count >= simulation->config.required_compiles)
	{
		coder->is_finished = true;
	}
	should_exit = (coder->has_burned_out || coder->is_finished
			|| simulation->is_stopped);
	if (should_exit)
		pthread_cond_broadcast(&simulation->state_changed);
	pthread_mutex_unlock(&simulation->state_mutex);
	return (should_exit);
}

static void	print_compile_start(t_coder *coder, t_ms timestamp)
{
	printf("%ld %d has taken a dongle\n", timestamp, coder->id);
	printf("%ld %d has taken a dongle\n", timestamp, coder->id);
	printf("%ld %d is compiling\n", timestamp, coder->id);
}

static bool	coder_start_compile(t_coder *coder)
{
	t_simulation	*simulation;
	t_ms			timestamp;

	simulation = coder->simulation;
	pthread_mutex_lock(&simulation->log_mutex);
	pthread_mutex_lock(&simulation->state_mutex);
	if (simulation->is_stopped || coder->has_burned_out)
	{
		pthread_mutex_unlock(&simulation->state_mutex);
		pthread_mutex_unlock(&simulation->log_mutex);
		return (false);
	}
	timestamp = simulation_elapsed_ms(simulation);
	coder->burnout_deadline_ms = timestamp + simulation->config.burnout_ms;
	coder->compile_count++;
	pthread_cond_broadcast(&simulation->state_changed);
	pthread_mutex_unlock(&simulation->state_mutex);
	print_compile_start(coder, timestamp);
	pthread_mutex_unlock(&simulation->log_mutex);
	return (true);
}

static bool	coder_work_cycle(t_coder *coder)
{
	t_simulation	*simulation;

	simulation = coder->simulation;
	if (!coder_acquire_dongles(coder))
		return (false);
	if (!coder_start_compile(coder) || !simulation_sleep(simulation,
			simulation->config.compile_ms))
	{
		coder_release_dongles(coder);
		return (false);
	}
	coder_release_dongles(coder);
	if (simulation_should_stop(simulation))
		return (false);
	simulation_log(simulation, coder->id, "is debugging");
	if (!simulation_sleep(simulation, simulation->config.debug_ms))
		return (false);
	simulation_log(simulation, coder->id, "is refactoring");
	return (simulation_sleep(simulation, simulation->config.refactor_ms));
}

void	*coder_thread(void *argument)
{
	t_coder	*coder;

	coder = (t_coder *)argument;
	simulation_wait_for_start(coder->simulation);
	while (!coder_should_exit(coder))
	{
		if (!coder_work_cycle(coder))
			break ;
	}
	coder_release_dongles(coder);
	return (NULL);
}
