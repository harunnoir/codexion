/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   time.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abait-el <abait-el@student.1337.ma>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/10 02:28:22 by abait-el          #+#    #+#             */
/*   Updated: 2026/09/10 02:28:22 by abait-el         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"
#include <sys/time.h>

t_ms	clock_now_ms(void)
{
	struct timeval	now;

	gettimeofday(&now, NULL);
	return ((t_ms)now.tv_sec * 1000 + (t_ms)now.tv_usec / 1000);
}

t_ms	simulation_elapsed_ms(t_simulation *simulation)
{
	return (clock_now_ms() - simulation->started_at_ms);
}

void	timespec_from_ms(struct timespec *timespec, t_ms absolute_ms)
{
	timespec->tv_sec = absolute_ms / 1000;
	timespec->tv_nsec = (absolute_ms % 1000) * 1000000L;
}
