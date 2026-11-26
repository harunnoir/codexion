/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   simulation.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abait-el <abait-el@student.1337.ma>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/10 02:28:16 by abait-el          #+#    #+#             */
/*   Updated: 2026/09/10 02:28:17 by abait-el         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <errno.h>
#include "codexion.h"

void	simulation_wait_for_start(t_simulation *simulation)
{
	pthread_mutex_lock(&simulation->state_mutex);
	while (!simulation->start_released)
		pthread_cond_wait(&simulation->state_changed,
			&simulation->state_mutex);
	pthread_mutex_unlock(&simulation->state_mutex);
}

bool	simulation_should_stop(t_simulation *simulation)
{
	bool	should_stop;

	pthread_mutex_lock(&simulation->state_mutex);
	should_stop = simulation->is_stopped;
	pthread_mutex_unlock(&simulation->state_mutex);
	return (should_stop);
}

bool	simulation_sleep(t_simulation *simulation, t_ms duration_ms)
{
	struct timespec	wake_time;
	int				status;
	bool			completed;

	timespec_from_ms(&wake_time, clock_now_ms() + duration_ms);
	pthread_mutex_lock(&simulation->state_mutex);
	status = 0;
	while (!simulation->is_stopped && status != ETIMEDOUT)
		status = pthread_cond_timedwait(&simulation->sleep_condition,
				&simulation->state_mutex, &wake_time);
	completed = !simulation->is_stopped;
	pthread_mutex_unlock(&simulation->state_mutex);
	return (completed);
}

void	simulation_run(t_simulation *simulation)
{
	int	index;

	pthread_join(simulation->monitor_thread, NULL);
	index = 0;
	while (index < simulation->config.coder_count)
	{
		pthread_join(simulation->coders[index].thread, NULL);
		index++;
	}
}
