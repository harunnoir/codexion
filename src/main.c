/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abait-el <abait-el@student.1337.ma>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/10 02:28:26 by abait-el          #+#    #+#             */
/*   Updated: 2026/09/10 02:28:26 by abait-el         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"
#include <stdio.h>

int	main(int argc, char **argv)
{
	t_config		config;
	t_simulation	simulation;

	if (config_parse(&config, argc, argv) != 0)
		return (1);
	if (simulation_init(&simulation, &config) != 0)
	{
		fprintf(stderr, "codexion: initialization failed\n");
		return (1);
	}
	simulation_run(&simulation);
	simulation_destroy(&simulation);
	return (0);
}
