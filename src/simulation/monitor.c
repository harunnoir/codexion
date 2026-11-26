/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   monitor.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abait-el <abait-el@student.1337.ma>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/10 02:28:14 by abait-el          #+#    #+#             */
/*   Updated: 2026/09/10 02:28:15 by abait-el         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <limits.h>
#include "codexion.h"

#define NO_DEADLINE LONG_MAX
#define BURNOUT_TOLERANCE_MS 9

static int	find_burned_out_coder(t_simulation *simulation)
{
	t_ms	now_ms;
	int		index;

	now_ms = simulation_elapsed_ms(simulation);
	index = 0;
	while (index < simulation->config.coder_count)
	{
		if (!simulation->coders[index].has_burned_out
			&& !simulation->coders[index].is_finished
			&& simulation->coders[index].burnout_deadline_ms
			+ BURNOUT_TOLERANCE_MS <= now_ms)
			return (index);
		index++;
	}
	return (-1);
}

static t_ms	find_next_deadline(t_simulation *simulation)
{
	t_ms	deadline_ms;
	int		index;

	deadline_ms = NO_DEADLINE;
	index = 0;
	while (index < simulation->config.coder_count)
	{
		if (!simulation->coders[index].has_burned_out
			&& !simulation->coders[index].is_finished
			&& simulation->coders[index].burnout_deadline_ms < deadline_ms)
			deadline_ms = simulation->coders[index].burnout_deadline_ms;
		index++;
	}
	return (deadline_ms);
}

static void	stop_simulation(t_simulation *simulation, int burned_index)
{
	int	coder_id;

	coder_id = -1;
	if (burned_index >= 0)
	{
		simulation->coders[burned_index].has_burned_out = true;
		coder_id = simulation->coders[burned_index].id;
	}
	simulation->is_stopped = true;
	pthread_cond_broadcast(&simulation->state_changed);
	pthread_cond_broadcast(&simulation->sleep_condition);
	pthread_mutex_unlock(&simulation->state_mutex);
	scheduler_wake_all(simulation);
	if (coder_id >= 0)
		simulation_log_burnout(simulation, coder_id);
}

static void	wait_until_deadline(t_simulation *simulation, t_ms deadline_ms)
{
	struct timespec	wake_time;

	timespec_from_ms(&wake_time, simulation->started_at_ms
		+ deadline_ms + BURNOUT_TOLERANCE_MS);
	pthread_cond_timedwait(&simulation->state_changed,
		&simulation->state_mutex, &wake_time);
}

void	*monitor_thread(void *argument)
{
	t_simulation	*simulation;
	t_ms			deadline_ms;
	int				burned_index;

	simulation = (t_simulation *)argument;
	simulation_wait_for_start(simulation);
	pthread_mutex_lock(&simulation->state_mutex);
	burned_index = -1;
	while (!simulation->is_stopped)
	{
		burned_index = find_burned_out_coder(simulation);
		if (burned_index >= 0)
			break ;
		deadline_ms = find_next_deadline(simulation);
		if (deadline_ms == NO_DEADLINE)
			break ;
		wait_until_deadline(simulation, deadline_ms);
	}
	stop_simulation(simulation, burned_index);
	return (NULL);
}
