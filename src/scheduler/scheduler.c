/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   scheduler.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abait-el <abait-el@student.1337.ma>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/10 02:27:49 by abait-el          #+#    #+#             */
/*   Updated: 2026/09/10 02:27:50 by abait-el         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static bool	coder_can_take_dongles(t_coder *coder)
{
	t_ms	now_ms;

	now_ms = clock_now_ms();
	if (!scheduler_pair_is_available(&coder->pending_request, now_ms))
		return (false);
	if (!scheduler_dongle_allows(coder->left_dongle, coder, now_ms))
		return (false);
	return (scheduler_dongle_allows(coder->right_dongle, coder, now_ms));
}

static void	grant_dongles(t_coder *coder)
{
	request_heap_remove(&coder->left_dongle->waiters, coder);
	request_heap_remove(&coder->right_dongle->waiters, coder);
	dongle_acquire(coder->left_dongle);
	dongle_acquire(coder->right_dongle);
	coder->has_pending_request = false;
	coder->has_dongles = true;
}

static bool	wait_for_dongles(t_coder *coder)
{
	t_simulation	*simulation;

	simulation = coder->simulation;
	while (!simulation_should_stop(simulation))
	{
		if (coder_can_take_dongles(coder))
		{
			grant_dongles(coder);
			return (true);
		}
		scheduler_wait_for_event(simulation);
	}
	return (false);
}

bool	coder_acquire_dongles(t_coder *coder)
{
	t_simulation	*simulation;
	bool			acquired;

	if (coder->left_dongle == coder->right_dongle)
		return (scheduler_wait_for_single(coder));
	simulation = coder->simulation;
	pthread_mutex_lock(&simulation->scheduler_mutex);
	if (!coder->has_pending_request && !scheduler_queue_request(coder))
	{
		pthread_mutex_unlock(&simulation->scheduler_mutex);
		return (false);
	}
	acquired = wait_for_dongles(coder);
	if (!acquired && coder->has_pending_request)
	{
		request_heap_remove(&coder->left_dongle->waiters, coder);
		request_heap_remove(&coder->right_dongle->waiters, coder);
		coder->has_pending_request = false;
	}
	pthread_mutex_unlock(&simulation->scheduler_mutex);
	return (acquired);
}

void	coder_release_dongles(t_coder *coder)
{
	t_simulation	*simulation;

	if (!coder->has_dongles)
		return ;
	simulation = coder->simulation;
	pthread_mutex_lock(&simulation->scheduler_mutex);
	dongle_release(coder->left_dongle, simulation->config.cooldown_ms);
	dongle_release(coder->right_dongle, simulation->config.cooldown_ms);
	coder->has_dongles = false;
	pthread_cond_broadcast(&simulation->scheduler_changed);
	pthread_mutex_unlock(&simulation->scheduler_mutex);
}
