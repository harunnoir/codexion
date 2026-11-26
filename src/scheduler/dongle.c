/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   dongle.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abait-el <abait-el@student.1337.ma>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/10 02:27:38 by abait-el          #+#    #+#             */
/*   Updated: 2026/09/10 02:27:39 by abait-el         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

bool	dongle_is_available(t_dongle *dongle, t_ms now_ms)
{
	bool	is_available;

	pthread_mutex_lock(&dongle->mutex);
	is_available = (!dongle->is_held && now_ms >= dongle->available_at_ms);
	pthread_mutex_unlock(&dongle->mutex);
	return (is_available);
}

void	dongle_acquire(t_dongle *dongle)
{
	pthread_mutex_lock(&dongle->mutex);
	dongle->is_held = true;
	pthread_mutex_unlock(&dongle->mutex);
}

void	dongle_release(t_dongle *dongle, t_ms cooldown_ms)
{
	pthread_mutex_lock(&dongle->mutex);
	dongle->is_held = false;
	dongle->available_at_ms = clock_now_ms() + cooldown_ms;
	pthread_mutex_unlock(&dongle->mutex);
}
