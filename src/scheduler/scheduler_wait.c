/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   scheduler_wait.c                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abait-el <abait-el@student.1337.ma>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/10 02:27:55 by abait-el          #+#    #+#             */
/*   Updated: 2026/09/10 02:27:56 by abait-el         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static t_ms	dongle_cooldown_end(t_dongle *dongle, t_ms now_ms)
{
	t_ms	available_at_ms;

	pthread_mutex_lock(&dongle->mutex);
	available_at_ms = 0;
	if (!dongle->is_held && dongle->available_at_ms > now_ms)
		available_at_ms = dongle->available_at_ms;
	pthread_mutex_unlock(&dongle->mutex);
	return (available_at_ms);
}

static t_ms	next_cooldown_end(t_simulation *simulation)
{
	t_ms	next_ms;
	t_ms	available_ms;
	t_ms	now_ms;
	int		index;

	now_ms = clock_now_ms();
	next_ms = 0;
	index = 0;
	while (index < simulation->config.coder_count)
	{
		available_ms = dongle_cooldown_end(&simulation->dongles[index], now_ms);
		if (available_ms && (!next_ms || available_ms < next_ms))
			next_ms = available_ms;
		index++;
	}
	return (next_ms);
}

void	scheduler_wait_for_event(t_simulation *simulation)
{
	struct timespec	wake_time;
	t_ms			wake_at_ms;

	wake_at_ms = next_cooldown_end(simulation);
	if (!wake_at_ms)
		pthread_cond_wait(&simulation->scheduler_changed,
			&simulation->scheduler_mutex);
	else
	{
		timespec_from_ms(&wake_time, wake_at_ms);
		pthread_cond_timedwait(&simulation->scheduler_changed,
			&simulation->scheduler_mutex, &wake_time);
	}
}

bool	scheduler_wait_for_single(t_coder *coder)
{
	t_simulation	*simulation;

	simulation = coder->simulation;
	pthread_mutex_lock(&simulation->scheduler_mutex);
	while (!simulation_should_stop(simulation))
		pthread_cond_wait(&simulation->scheduler_changed,
			&simulation->scheduler_mutex);
	pthread_mutex_unlock(&simulation->scheduler_mutex);
	return (false);
}

void	scheduler_wake_all(t_simulation *simulation)
{
	pthread_mutex_lock(&simulation->scheduler_mutex);
	pthread_cond_broadcast(&simulation->scheduler_changed);
	pthread_mutex_unlock(&simulation->scheduler_mutex);
}
