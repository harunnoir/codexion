/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   destroy.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abait-el <abait-el@student.1337.ma>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/10 02:28:06 by abait-el          #+#    #+#             */
/*   Updated: 2026/09/10 02:28:07 by abait-el         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <stdlib.h>
#include "codexion.h"

void	simulation_destroy_sync(t_simulation *simulation)
{
	pthread_mutex_destroy(&simulation->log_mutex);
	pthread_mutex_destroy(&simulation->state_mutex);
	pthread_cond_destroy(&simulation->state_changed);
	pthread_cond_destroy(&simulation->sleep_condition);
	pthread_mutex_destroy(&simulation->scheduler_mutex);
	pthread_cond_destroy(&simulation->scheduler_changed);
}

void	simulation_destroy(t_simulation *simulation)
{
	int	index;

	index = 0;
	while (index < simulation->config.coder_count)
		pthread_mutex_destroy(&simulation->dongles[index++].mutex);
	free(simulation->dongles);
	free(simulation->coders);
	simulation->dongles = NULL;
	simulation->coders = NULL;
	simulation_destroy_sync(simulation);
}
