/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   log.c                                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abait-el <abait-el@student.1337.ma>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/10 02:28:20 by abait-el          #+#    #+#             */
/*   Updated: 2026/09/10 02:28:21 by abait-el         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"
#include <stdio.h>

void	simulation_log(t_simulation *simulation, int coder_id,
		const char *state)
{
	bool	can_print;

	pthread_mutex_lock(&simulation->log_mutex);
	pthread_mutex_lock(&simulation->state_mutex);
	can_print = !simulation->is_stopped;
	pthread_mutex_unlock(&simulation->state_mutex);
	if (can_print)
	{
		printf("%ld %d %s\n", simulation_elapsed_ms(simulation), coder_id,
			state);
	}
	pthread_mutex_unlock(&simulation->log_mutex);
}

void	simulation_log_burnout(t_simulation *simulation, int coder_id)
{
	pthread_mutex_lock(&simulation->log_mutex);
	printf("%ld %d burned out\n", simulation_elapsed_ms(simulation), coder_id);
	pthread_mutex_unlock(&simulation->log_mutex);
}
