/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   init_sync.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abait-el <abait-el@student.1337.ma>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/10 02:28:10 by abait-el          #+#    #+#             */
/*   Updated: 2026/09/10 02:28:11 by abait-el         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static int	init_state_sync(t_simulation *simulation)
{
	if (pthread_mutex_init(&simulation->state_mutex, NULL) != 0)
		return (-1);
	if (pthread_cond_init(&simulation->state_changed, NULL) != 0)
	{
		pthread_mutex_destroy(&simulation->state_mutex);
		return (-1);
	}
	if (pthread_cond_init(&simulation->sleep_condition, NULL) != 0)
	{
		pthread_cond_destroy(&simulation->state_changed);
		pthread_mutex_destroy(&simulation->state_mutex);
		return (-1);
	}
	return (0);
}

static int	init_scheduler_sync(t_simulation *simulation)
{
	if (pthread_mutex_init(&simulation->scheduler_mutex, NULL) != 0)
		return (-1);
	if (pthread_cond_init(&simulation->scheduler_changed, NULL) != 0)
	{
		pthread_mutex_destroy(&simulation->scheduler_mutex);
		return (-1);
	}
	return (0);
}

int	simulation_init_sync(t_simulation *simulation)
{
	if (pthread_mutex_init(&simulation->log_mutex, NULL) != 0)
		return (-1);
	if (init_state_sync(simulation) != 0)
	{
		pthread_mutex_destroy(&simulation->log_mutex);
		return (-1);
	}
	if (init_scheduler_sync(simulation) != 0)
	{
		pthread_cond_destroy(&simulation->sleep_condition);
		pthread_cond_destroy(&simulation->state_changed);
		pthread_mutex_destroy(&simulation->state_mutex);
		pthread_mutex_destroy(&simulation->log_mutex);
		return (-1);
	}
	return (0);
}
