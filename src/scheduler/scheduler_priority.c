/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   scheduler_priority.c                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abait-el <abait-el@student.1337.ma>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/10 02:27:52 by abait-el          #+#    #+#             */
/*   Updated: 2026/09/10 02:27:53 by abait-el         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static t_request	create_request(t_coder *coder)
{
	t_request		request;
	t_simulation	*simulation;

	simulation = coder->simulation;
	request.owner = coder;
	request.arrival_order = simulation->next_request_order++;
	pthread_mutex_lock(&simulation->state_mutex);
	request.burnout_deadline_ms = coder->burnout_deadline_ms;
	pthread_mutex_unlock(&simulation->state_mutex);
	return (request);
}

bool	scheduler_queue_request(t_coder *coder)
{
	t_request	request;

	request = create_request(coder);
	if (!request_heap_push(&coder->left_dongle->waiters, request))
		return (false);
	if (!request_heap_push(&coder->right_dongle->waiters, request))
	{
		request_heap_remove(&coder->left_dongle->waiters, coder);
		return (false);
	}
	coder->pending_request = request;
	coder->has_pending_request = true;
	pthread_cond_broadcast(&coder->simulation->scheduler_changed);
	return (true);
}

bool	scheduler_pair_is_available(t_request *request, t_ms now_ms)
{
	t_coder	*coder;

	coder = request->owner;
	if (!dongle_is_available(coder->left_dongle, now_ms))
		return (false);
	return (dongle_is_available(coder->right_dongle, now_ms));
}

static bool	request_has_time_to_wait(t_request *request, t_ms now_ms)
{
	t_simulation	*simulation;
	t_ms			blocked_until;
	t_ms			deadline;

	simulation = request->owner->simulation;
	blocked_until = now_ms + simulation->config.compile_ms
		+ simulation->config.cooldown_ms;
	deadline = simulation->started_at_ms + request->burnout_deadline_ms;
	return (deadline >= blocked_until);
}

bool	scheduler_dongle_allows(t_dongle *dongle, t_coder *coder,
		t_ms now_ms)
{
	t_request	first;

	if (!request_heap_peek(&dongle->waiters, &first))
		return (true);
	if (first.owner == coder)
		return (true);
	if (dongle->waiters.compare(&first, &coder->pending_request) >= 0)
		return (true);
	if (scheduler_pair_is_available(&first, now_ms))
		return (false);
	return (request_has_time_to_wait(&first, now_ms));
}
